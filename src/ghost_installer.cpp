#include <windows.h>
#include <shlobj_core.h>
#include <shlwapi.h>

#include "my-gists/ukagaka/SSP_Runner.hpp"
#include "my-gists/windows/LoadStringFromResource.hpp"

#include "resource/resource.h"
#include "ghost_installer.hpp"

std::wstring download_temp_file(const std::wstring& url, const std::wstring& file_suffix);
std::wstring get_ghost_url(){
	#ifdef _DEBUG
	return L"https://github.com/Taromati2/Taromati2/releases/download/balloon/wiz.nar";
	#else
	//TODO get ghost url from self file
	#endif
}

namespace ssp_install {
	std::wstring program_dir;
}

LRESULT CALLBACK InstallPathSelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lp) {
	using namespace ssp_install;
	switch(message) {
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hDlg, IDC_PATHEDIT));
		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK: {
			// Get number of characters.
			size_t pathlen = SendDlgItemMessage(hDlg,
												   IDC_PATHEDIT,
												   EM_LINELENGTH,
												   (WPARAM)0,
												   (LPARAM)0);
			if(pathlen == 0) {
				MessageBox(hDlg,
						   L"No characters entered.",
						   L"Error",
						   MB_OK);

				EndDialog(hDlg, TRUE);
				return FALSE;
			}

			// Put the number of characters into first word of buffer.
			program_dir.resize(pathlen);
			program_dir[0] = (wchar_t)pathlen;

			// Get the characters.
			SendDlgItemMessage(hDlg,
							   IDC_PATHEDIT,
							   EM_GETLINE,
							   (WPARAM)0,		// line 0
							   (LPARAM)program_dir.data());

			//make sure the path is valid
			if(!PathIsDirectory(program_dir.c_str())) {
				//Check if the parent directory exists
				std::wstring parent_dir = program_dir.substr(0, program_dir.find_last_of(L"\\/"));
				if(PathIsDirectory(parent_dir.c_str())) {
					//Create the directory
					if(!CreateDirectory(program_dir.c_str(), NULL)) {
						MessageBox(hDlg,
								   L"Could not create directory.",
								   L"Error",
								   MB_OK);
						EndDialog(hDlg, TRUE);
						return FALSE;
					}
				}
				else {
					MessageBox(hDlg,
							   L"The path is not a valid directory.",
							   L"Error",
							   MB_OK);

					EndDialog(hDlg, TRUE);
					return FALSE;
				}
			}
			
			EndDialog(hDlg, TRUE);
			return TRUE; 
		}
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			exit(0);
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
}

// Winmain
int APIENTRY WinMain(
	_In_ HINSTANCE	   hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR		   lpCmdLine,
	_In_ int		   nShowCmd) {

	SSP_Runner SSP;
	if(SSP.IsInstalled()) {
		try{
			auto nar_file=download_temp_file(get_ghost_url(), L".nar");
			SSP.install_nar(nar_file);
			//We can't wait for ssp to terminate before deleting the nar file, because when ghost ends is up to the user
			//So, no clearing of temporary files
		}
		catch(const std::exception& e) {
			MessageBoxA(NULL, e.what(), "Error", MB_OK);
		}
	}
	else{
		//download and install SSP
		auto ssp_file=download_temp_file(L"http://ssp.shillest.net/archive/redir.cgi?stable&full", L".exe");
		EXE_Runner SSP_EXE(ssp_file);
		//get language id
		int lang_id=0;
		{
			wchar_t lang_id_str[5]={0};
			GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, lang_id_str, 5);
			lang_id=_wtoi(lang_id_str);
		}
		//show install path dialog
		std::wstring program_dir=DefaultSSPinstallPath();
		auto		 install_path_scl_ui = CreateDialogW(hInstance, (LPCTSTR)IDD_INSTALLATION_PATH_SELECTION, NULL, (DLGPROC)InstallPathSelDlgProc);
		ShowWindow(install_path_scl_ui, SW_SHOW);
		MSG			 msg;
		while(GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//install SSP(download zip & extract)
		SSP_EXE.RunAndWait(L"-o\"" + program_dir + L"\"", L"-y");
		//Delete temporary files
		DeleteFileW(ssp_file.c_str());
		//chose&install language pack & ghost for starter
		//install ghost
	}
	return 0;
}

std::wstring DefaultSSPinstallPath() {
	#ifdef _DEBUG
		std::wstring self_path;
		self_path.resize(MAX_PATH);
		GetModuleFileNameW(NULL, self_path.data(), MAX_PATH);
		self_path = self_path.substr(0, self_path.find_last_of(L"\\"));
		self_path += L"\\SSP";
		return self_path;
	#else
		std::wstring program_dir;
		program_dir.resize(MAX_PATH);
		SHGetSpecialFolderPathW(NULL, program_dir.data(), CSIDL_PROGRAM_FILESX86, FALSE);
		program_dir.resize(wcslen(program_dir.data()));
		program_dir += L"\\SSP";
	#endif
}
