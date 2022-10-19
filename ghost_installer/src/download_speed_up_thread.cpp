#include <windows.h>

#include "my-gists/windows/LoadStringFromResource.hpp"
#include "my-gists/windows/download_file.hpp"
#include "my-gists/windows/get_temp_path.hpp"
#include "my-gists/STL/yaml_reader.hpp"

#include "resource/resource.h"

namespace download_speed_up_thread {
	void download_speed_up_ssp() {
		try {
			download_file_to_temp_dir(L"https://ssp.shillest.net/archive/redir.cgi?stable&prog", L"SSP.exe");
		}
		catch(const std::exception& e) {
			MessageBoxA(NULL, e.what(), "Error", MB_OK);
			throw;
		}
	}
	void download_speed_up_nar() {
		try {
			download_file_to_temp_dir(LoadCStringFromResource(IDS_NAR_URL), LoadCStringFromResource(IDS_NAR_FILE_NAME));
		}
		catch(const std::exception& e) {
			MessageBoxA(NULL, e.what(), "Error", MB_OK);
			throw;
		}
	}
	std::wstring GetLangPackUrl(LANGID langid) {
		yaml_reader langidyaml_reader;
		langidyaml_reader.read_url(L"https://raw.githubusercontent.com/ukatech/ssp-langlist/main/lang.yml");
		auto langidyaml = langidyaml_reader.find(L"langid", std::to_wstring(langid));
		return langidyaml[L"url"];
	}
	void download_speed_up_langpack() {
		try {
			auto langpackfile = std::wstring(get_temp_path()) + L"langpack.nar";
			if(_waccess(langpackfile.c_str(), 0) == 0) {
				return;
			}
			//get language id
			auto langid		 = GetUserDefaultUILanguage();
			auto langpackurl = GetLangPackUrl(langid);
			if(!langpackurl.empty())
				download_file(langpackurl, langpackfile);
		}
		catch(const std::exception& e) {
			MessageBoxA(NULL, e.what(), "Error", MB_OK);
			throw;
		}
	}
}		// namespace download_speed_up_thread
