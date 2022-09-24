#include <windows.h>

#include "my-gists/windows/LoadStringFromResource.hpp"
#include "my-gists/windows/download_file.hpp"

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
			download_file_to_temp_dir(L"https://github.com/Taromati2/package-factory/releases/latest/download/Taromati2.nar", L"Taromati2.nar");
		}
		catch(const std::exception& e) {
			MessageBoxA(NULL, e.what(), "Error", MB_OK);
			throw;
		}
	}
}		// namespace download_speed_up_thread
