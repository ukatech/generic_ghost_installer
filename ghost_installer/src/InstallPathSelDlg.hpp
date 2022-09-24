#include <windows.h>
#include <string>

namespace ssp_install {
	inline std::wstring program_dir;
	inline bool			ok_to_install = false;
}		// namespace ssp_install
LRESULT CALLBACK InstallPathSelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lp);
