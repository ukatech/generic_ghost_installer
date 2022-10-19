#include <windows.h>
#include <shlobj_core.h>
#include <shlwapi.h>

#include "my-gists/ukagaka/SSP_Runner.hpp"
#include "my-gists/windows/LoadStringFromResource.hpp"
#include "my-gists/windows/WaitForXObjectWithMessageLoop.hpp"
#include "my-gists/windows/get_temp_path.hpp"
#include "my-gists/windows/download_file.hpp"
#include "my-gists/windows/IsNetworkHasCost.hpp"

#include "resource/resource.h"

#include "download_speed_up_thread.hpp"
#include "InstallPathSelDlg.hpp"
#include "ghost_installer.hpp"

// Winmain
int APIENTRY WinMain(
	_In_ HINSTANCE	   hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR		   lpCmdLine,
	_In_ int		   nShowCmd) {
	if(IsNetworkHasCost()) {
		auto result = MessageBox(NULL,
								 LoadCStringFromResource(IDS_NETWORK_HAS_COST),
								 LoadCStringFromResource(IDS_NOTICE_TITLE),
								 MB_YESNO);
		if(result == IDNO)
			return 0;
	}

	SSP_Runner SSP;
	HANDLE	   nar_download_thread = NULL;
	auto	   downloading_ui	   = CreateDialogW(hInstance, (LPCTSTR)IDD_DOWNLOADING, NULL, NULL);
	//lambda for start downloading thread
	auto start_download_thread = [&](HANDLE& handle,void(*download_thread_func)()) {
		handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_thread_func, NULL, 0, NULL);
		if(!handle) {
			MessageBox(NULL,
					   LoadCStringFromResource(IDS_FAILED_TO_CREATE_DL_PROCESS),
					   LoadCStringFromResource(IDS_ERROR_TITLE),
					   MB_OK);
			exit(1);
		}
	};
	auto start_nar_download_thread = [&]() {
		start_download_thread(nar_download_thread, download_speed_up_thread::download_speed_up_nar);
	};
	//lambda for show downloading ui and wait for downloading thread
	auto wait_for = [&](HANDLE some_download_thread) {
		ShowWindow(downloading_ui, SW_SHOW);
		DWORD wait_result = WaitForSingleObjectWithMessageLoop(some_download_thread, INFINITE);
		ShowWindow(downloading_ui, SW_HIDE);
		if(wait_result == WAIT_FAILED) {
			MessageBox(NULL, LoadCStringFromResource(IDS_FAILED_TO_DOWNLOAD_REQUIRED_FILE), LoadCStringFromResource(IDS_ERROR_TITLE), MB_OK);
			exit(1);
		}
	};
	//lambda for check thread success
	auto is_success = [&](HANDLE some_download_thread) {
		DWORD wait_result = WaitForSingleObject(some_download_thread, 0);
		return wait_result == WAIT_OBJECT_0;
	};
	
	if(SSP.IsInstalled()) {
		start_nar_download_thread();
	install_ghost:
		auto nar_file = std::wstring(get_temp_path()) + LoadCStringFromResource(IDS_NAR_FILE_NAME);
		wait_for(nar_download_thread);
		#ifndef _DEBUG
			SSP.install_nar_and_delete_source_if_succes(nar_file);
		#else
			SSP.install_nar(nar_file);
		#endif
	}
	else {
		auto ssp_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_ssp, NULL, 0, NULL);
		HANDLE lang_pack_download_thread = NULL;
		//lambda for start downloading thread
		auto start_lang_pack_download_thread = [&]() {
			start_download_thread(lang_pack_download_thread, download_speed_up_thread::download_speed_up_langpack);
		};
		auto response = MessageBox(NULL,
								   LoadCStringFromResource(IDS_SSP_NOT_INSTALLED_INFO),
								   LoadCStringFromResource(IDS_SSP_NOT_INSTALLED_TITLE),
								   MB_YESNO);
		if(response != IDYES)
			return 0;
		{
			if(is_success(ssp_download_thread)) {
				//start lang pack download
				start_lang_pack_download_thread();
			}
			auto	   ssp_file = std::wstring(get_temp_path()) + L"SSP.exe";
			EXE_Runner SSP_EXE(ssp_file);
			//show install path dialog
			ssp_install::program_dir = DefaultSSPinstallPath();
			auto install_path_scl_ui = CreateDialogW(hInstance, (LPCTSTR)IDD_INSTALLATION_PATH_SELECTION, NULL, (DLGPROC)InstallPathSelDlgProc);
			ShowWindow(install_path_scl_ui, SW_SHOW);
			{
				MSG msg;
				while(GetMessage(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					if(ssp_install::ok_to_install)
						break;
				}
			}
			DestroyWindow(install_path_scl_ui);
			//create installation directory
			switch(SHCreateDirectoryExW(NULL, ssp_install::program_dir.c_str(), NULL)) {
			case ERROR_ALREADY_EXISTS:
			case ERROR_SUCCESS:
			case ERROR_FILE_EXISTS: {
				//wait for ssp to download
				wait_for(ssp_download_thread);
				//close the ssp download thread
				CloseHandle(ssp_download_thread);
				//start nar download
				if(!lang_pack_download_thread)
					start_lang_pack_download_thread();
				else if(is_success(lang_pack_download_thread))
					start_nar_download_thread();
				//install SSP
				SSP_EXE.RunAndWait(L"-o\"" + ssp_install::program_dir + L"\"", L"-y");
				break;
			}
			default:
				MessageBoxW(NULL, LoadCStringFromResource(IDS_FAILED_TO_CREATE_INSTALLATION_FOLDER), LoadCStringFromResource(IDS_ERROR_TITLE), MB_OK);
				return 1;
			}
			//set SSP_Runner's path
			SSP.reset_path(ssp_install::program_dir + L"\\ssp.exe");
			if(!SSP.IsInstalled()) {
				MessageBoxW(NULL, LoadCStringFromResource(IDS_FAILED_TO_INSTALL_SSP), LoadCStringFromResource(IDS_ERROR_TITLE), MB_OK);
				return 1;
			}
			#ifndef _DEBUG
			//Delete temporary files
			DeleteFileW(ssp_file.c_str());
			#endif
		}
		auto langpackfile = std::wstring(get_temp_path()) + L"langpack.nar";
		//wait 
		wait_for(lang_pack_download_thread);
		CloseHandle(lang_pack_download_thread);
		if(!nar_download_thread)
			start_nar_download_thread();
		//install language pack if exists
		if(_waccess(langpackfile.c_str(), 0) == 0)
			#ifndef _DEBUG
				SSP.install_nar_and_delete_source_if_succes(langpackfile);
			#else
				SSP.install_nar(langpackfile);
			#endif
		//install ghost
		goto install_ghost;
	}
	//close things
	DestroyWindow(downloading_ui);
	CloseHandle(nar_download_thread);
	return 0;
}
