#include <windows.h>
#include <ShlObj_core.h>

#include "my-gists/windows/LoadStringFromResource.hpp"

#include "resource/resource.h"
#include "InstallPathSelDlg.hpp"

bool get_edit_Dia_text_as_path(std::wstring& path, HWND hDlg, WORD IDC) {
	// Get number of characters.
	size_t pathlen = SendDlgItemMessage(hDlg,
										IDC,
										EM_LINELENGTH,
										(WPARAM)0,
										(LPARAM)0);
	if(pathlen == 0) {
		MessageBox(hDlg,
				   LoadCStringFromResource(IDS_PLEASE_ENTER_THE_PATH),
				   LoadCStringFromResource(IDS_ERROR_TITLE),
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
				bi.lpszTitle	  = LoadCStringFromResource(IDS_SELECT_THE_SSP_INSTALLATION_PATH);
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
