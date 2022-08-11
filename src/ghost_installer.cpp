#include <windows.h>
#include <shlobj_core.h>
#include <shlwapi.h>

#include "my-gists/ukagaka/SSP_Runner.hpp"
#include "my-gists/windows/LoadStringFromResource.hpp"

#include "resource/resource.h"
#include "ghost_installer.hpp"

//shlwapi.lib
#pragma comment(lib, "shlwapi.lib")

std::wstring download_temp_file(const std::wstring& url, const std::wstring& file_suffix);
std::wstring get_ghost_url() {
	#ifdef _DEBUG
		return L"https://github.com/Taromati2/Taromati2/releases/download/balloon/wiz.nar";
	#else
		//TODO get ghost url from self file
	#endif
}

namespace ssp_install {
	std::wstring program_dir;
	bool		 ok_to_install = false;
}		// namespace ssp_install
bool get_edit_Dia_text_as_path(std::wstring& path, HWND hDlg, WORD IDC) {
	// Get number of characters.
	size_t pathlen = SendDlgItemMessage(hDlg,
										IDC,
										EM_LINELENGTH,
										(WPARAM)0,
										(LPARAM)0);
	if(pathlen == 0) {
		MessageBox(hDlg,
				   L"No characters entered.",
				   L"Error",
				   MB_OK);
		return false;
	}

	// Put the number of characters into first word of buffer.
	path.resize(pathlen);
	path[0] = (wchar_t)pathlen;

	// Get the characters.
	SendDlgItemMessage(hDlg,
					   IDC,
					   EM_GETLINE,
					   (WPARAM)0,		// line 0
					   (LPARAM)path.data());
	return true;
}
LRESULT CALLBACK InstallPathSelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lp) {
	using namespace ssp_install;
	switch(message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_PATHEDIT, program_dir.c_str());
		SetFocus(GetDlgItem(hDlg, IDC_PATHEDIT));
		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK: {
			if(get_edit_Dia_text_as_path(program_dir, hDlg, IDC_PATHEDIT)) {
				EndDialog(hDlg, TRUE);
				ok_to_install = true;
				return TRUE;
			}
			else
				return FALSE;
		}
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			exit(0);
		case IDSELECT:
			//get path by browsing
			//set browsing Root as IDC_PATHEDIT's text
			{
				BROWSEINFO bi;
				ZeroMemory(&bi, sizeof(bi));
				bi.hwndOwner	  = hDlg;
				bi.pszDisplayName = new wchar_t[MAX_PATH];
				bi.lpszTitle	  = L"Select installation path";
				bi.ulFlags		  = BIF_RETURNONLYFSDIRS;
				LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
				if(pidl != NULL) {
					SHGetPathFromIDList(pidl, bi.pszDisplayName);
					program_dir = bi.pszDisplayName;
					if(!program_dir.ends_with(L"SSP") || !program_dir.ends_with(L"SSP\\"))
						program_dir += L"\\SSP\\";
					SetDlgItemText(hDlg, IDC_PATHEDIT, program_dir.c_str());
				}
				delete[] bi.pszDisplayName;
				return TRUE;
			}
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
		try {
			auto nar_file = download_temp_file(get_ghost_url(), L".nar");
			SSP.install_nar(nar_file);
			//We can't wait for ssp to terminate before deleting the nar file, because when ghost ends is up to the user
			//So, no clearing of temporary files
		}
		catch(const std::exception& e) {
			MessageBoxA(NULL, e.what(), "Error", MB_OK);
		}
	}
	else {
		//download and install SSP
		auto	   ssp_file = download_temp_file(L"http://ssp.shillest.net/archive/redir.cgi?stable&full", L".exe");
		EXE_Runner SSP_EXE(ssp_file);
		//get language id
		int lang_id = 0;
		{
			wchar_t lang_id_str[5] = {0};
			GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, lang_id_str, 5);
			lang_id = _wtoi(lang_id_str);
		}
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
		//create installation directory
		switch(SHCreateDirectoryExW(NULL, ssp_install::program_dir.c_str(), NULL)) {
		case ERROR_ALREADY_EXISTS:
		case ERROR_SUCCESS:
		case ERROR_FILE_EXISTS:
			//install SSP
			SSP_EXE.RunAndWait(L"-o\"" + ssp_install::program_dir + L"\"", L"-y");
			//Delete temporary files
			DeleteFileW(ssp_file.c_str());
			break;
		default:
			//Delete temporary files
			DeleteFileW(ssp_file.c_str());
			MessageBoxW(NULL, L"Could not create installation directory.", L"Error", MB_OK);
			return 1;
		}
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
		return program_dir;
	#endif
}
