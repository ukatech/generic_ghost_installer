#include <windows.h>
#include <shlobj_core.h>
#include "my-gists/ukagaka/SSP_Runner.hpp"
#include "my-gists/windows/LoadStringFromResource.hpp"

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
		}
		catch(const std::exception& e) {
			MessageBoxA(NULL, e.what(), "Error", MB_OK);
		}
	}
	else{
		//download and install SSP
		auto ssp_file=download_temp_file(L"http://ssp.shillest.net/archive/redir.cgi?stable&full", L".exe");
		EXE_Runner SSP_EXE(ssp_file);
		//Get the program (x86) directory for installation
		std::wstring program_dir;
		program_dir.reserve(MAX_PATH);
		SHGetSpecialFolderPathW(NULL, program_dir.data(), CSIDL_PROGRAM_FILESX86, FALSE);
		program_dir.resize(wcslen(program_dir.data()));
		program_dir+=L"\\SSP";
		SSP_EXE(L"-o\""+program_dir+L"\"");
		//get language id
		//show install path dialog
		//install SSP(download zip & extract)
		//chose&install language pack & ghost for starter
		//install ghost
	}
	return 0;
}
