#include <windows.h>
#include <shlobj_core.h>
#include "my-gists/ukagaka/SSP_Runner.hpp"
#include "my-gists/windows/LoadStringFromResource.hpp"
#include "ghost_installer.hpp"

std::wstring download_temp_file(const std::wstring& url, const std::wstring& file_suffix);
std::wstring get_ghost_url(){
	#ifdef _DEBUG
	return L"https://github.com/Taromati2/Taromati2/releases/download/balloon/wiz.nar";
	#else
	//TODO get ghost url from self file
	#endif
}

int main() {
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
		// program_dir = SOME_MAIGC;
		
		//install SSP(download zip & extract)
		SSP_EXE.RunAndWait(L"-o\"" + program_dir + L"\"", L"-y");
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
