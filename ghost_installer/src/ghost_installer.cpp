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
	//*
	if(IsNetworkHasCost()) {
		auto result = MessageBox(NULL,
								 L"您的网络可能是计费网络，是否继续安装？",
								 L"注意",
								 MB_YESNO);
		if(result == IDNO)
			return 0;
	}
	//*/

	SSP_Runner SSP;
	HANDLE	   nar_download_thread = NULL;
	auto	   downloading_ui	   = CreateDialogW(hInstance, (LPCTSTR)IDD_DOWNLOADING, NULL, NULL);
	if(SSP.IsInstalled()) {
		nar_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_nar, NULL, 0, NULL);
	install_ghost:
		if(!nar_download_thread) {
			MessageBox(NULL,
					   L"创建下载进程失败",
					   L"Error",
					   MB_OK);
			return 1;
		}
		ShowWindow(downloading_ui, SW_SHOW);
		DWORD wait_result = WaitForSingleObjectWithMessageLoop(nar_download_thread, INFINITE);
		ShowWindow(downloading_ui, SW_HIDE);
		if(wait_result == WAIT_FAILED) {
			MessageBox(NULL, L"下载nar失败", L"Error", MB_OK);
			exit(1);
		}
		auto nar_file = std::wstring(get_temp_path()) + L"Taromati2.nar";
		SSP.install_nar(nar_file);
		//We can't wait for ssp to terminate before deleting the nar file, because when ghost ends is up to the user
		//So, no clearing of temporary files
	}
	else {
		auto ssp_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_ssp, NULL, 0, NULL);
		HANDLE lang_pack_download_thread = NULL;
		if(!ssp_download_thread) {
			MessageBox(NULL,
					   L"创建下载进程失败",
					   L"Error",
					   MB_OK);
			return 0;
		}
		auto response = MessageBox(NULL,
								   L"SSP未安装，点击确认以安装SSP并继续\n若已安装SSP，请更新到最新版后右键人格->设置->选项->常规->关联相关文件扩展名，并重启此程序",
								   L"SSP未安装",
								   MB_YESNO);
		if(response != IDYES)
			return 0;
		{
			//wait for ssp to download
			DWORD wait_result = WaitForSingleObjectWithMessageLoop(ssp_download_thread, 0);
			if(wait_result == WAIT_OBJECT_0) {
				//start lang pack download
				lang_pack_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_langpack, NULL, 0, NULL);
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
				ShowWindow(downloading_ui, SW_SHOW);
				DWORD wait_result = WaitForSingleObjectWithMessageLoop(ssp_download_thread, INFINITE);
				ShowWindow(downloading_ui, SW_HIDE);
				if(wait_result != WAIT_OBJECT_0) {
					MessageBox(NULL, L"SSP下载失败", L"SSP下载失败", MB_OK);
					return 1;
				}
				//close the ssp download thread
				CloseHandle(ssp_download_thread);
				//start nar download
				if(!lang_pack_download_thread)
					lang_pack_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_langpack, NULL, 0, NULL);
				else {
					wait_result = WaitForSingleObjectWithMessageLoop(lang_pack_download_thread, 0);
					if(wait_result == WAIT_OBJECT_0)
						nar_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_nar, NULL, 0, NULL);
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
			//Delete temporary files
			DeleteFileW(ssp_file.c_str());
		}
		//wait 
		ShowWindow(downloading_ui, SW_SHOW);
		DWORD wait_result = WaitForSingleObjectWithMessageLoop(lang_pack_download_thread, INFINITE);
		ShowWindow(downloading_ui, SW_HIDE);
		if(wait_result == WAIT_OBJECT_0)
			nar_download_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)download_speed_up_thread::download_speed_up_nar, NULL, 0, NULL);
		else {
			MessageBoxW(NULL, L"语言包下载失败", L"Error", MB_OK);
			return 1;
		}
		//install language pack
		auto langpackfile = std::wstring(get_temp_path()) + L"langpack.nar";
		if(_waccess(langpackfile.c_str(), 0) == 0)
			SSP.install_nar(langpackfile);
		//install ghost
		goto install_ghost;
	}
	return 0;
}
