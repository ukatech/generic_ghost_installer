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
								 L"您的网络可能是计费网络，是否继续安装？",
								 L"注意",
								 MB_YESNO);
		if(result == IDNO)
			return 0;
	}

	SSP_Runner SSP;
	HANDLE	   nar_download_thread = NULL;
	auto	   downloading_ui	   = CreateDialogW(hInstance, (LPCTSTR)IDD_DOWNLOADING, NULL, NULL);
	//lambda for start downloading thread
	auto start_nar_download_thread = [&]() {
		nar_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_nar, NULL, 0, NULL);
	};
	//lambda for show downloading ui and wait for downloading thread
	auto wait_for = [&](HANDLE some_download_thread) {
		ShowWindow(downloading_ui, SW_SHOW);
		DWORD wait_result = WaitForSingleObjectWithMessageLoop(some_download_thread, INFINITE);
		ShowWindow(downloading_ui, SW_HIDE);
		if(wait_result == WAIT_FAILED) {
			MessageBox(NULL, L"下载必须的文件失败", L"Error", MB_OK);
			exit(1);
		}
	};
	
	if(SSP.IsInstalled()) {
		start_nar_download_thread();
	install_ghost:
		if(!nar_download_thread) {
			MessageBox(NULL,
					   L"创建下载进程失败",
					   L"Error",
					   MB_OK);
			return 1;
		}
		wait_for(nar_download_thread);
		auto nar_file = std::wstring(get_temp_path()) + L"Taromati2.nar";
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
			lang_pack_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_langpack, NULL, 0, NULL);
		};
		if(!ssp_download_thread) {
			MessageBox(NULL,
					   L"创建下载进程失败",
					   L"Error",
					   MB_OK);
			return 0;
		}
		auto response = MessageBox(NULL,
								   L"SSP未安装，此程序是运行ghost的基础平台\n点击确认以安装SSP并继续\n若已安装SSP，请关掉SSP并在接下来的安装路径选择中选择现有的SSP路径以更新SSP至最新版本",
								   L"SSP未安装",
								   MB_YESNO);
		if(response != IDYES)
			return 0;
		{
			//wait for ssp to download
			DWORD wait_result = WaitForSingleObject(ssp_download_thread, 0);
			if(wait_result == WAIT_OBJECT_0) {
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
				else {
					wait_result = WaitForSingleObject(lang_pack_download_thread, 0);
					if(wait_result == WAIT_OBJECT_0)
						start_nar_download_thread();
				}
				//install SSP
				SSP_EXE.RunAndWait(L"-o\"" + ssp_install::program_dir + L"\"", L"-y");
				break;
			}
			default:
				MessageBoxW(NULL, L"未能创建安装文件夹\n请考虑以管理员运行此程序或检查安装路径", L"Error", MB_OK);
				return 1;
			}
			//set SSP_Runner's path
			SSP.reset_path(ssp_install::program_dir + L"\\ssp.exe");
			if(!SSP.IsInstalled()) {
				MessageBoxW(NULL, L"未能安装SSP", L"Error", MB_OK);
				return 1;
			}
			#ifndef _DEBUG
			//Delete temporary files
			DeleteFileW(ssp_file.c_str());
			#endif
		}
		//wait 
		wait_for(lang_pack_download_thread);
		CloseHandle(lang_pack_download_thread);
		start_nar_download_thread();
		//install language pack
		auto langpackfile = std::wstring(get_temp_path()) + L"langpack.nar";
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
