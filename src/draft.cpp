#include <windows.h>
#include "my-gists/ukagaka/SSPpath.hpp"

std::wstring download_temp_file(const std::wstring& url, const std::wstring& file_suffix) {
    //get temp path
    static wchar_t temp_path[MAX_PATH];
    static bool temp_path_initer=(bool)GetTempPath(MAX_PATH, temp_path);
    //generate a temporary file name
    std::wstring temp_file_name;
    temp_file_name.reserve(MAX_PATH);
    GetTempFileNameW(temp_path, L"ghi", 0, temp_file_name.data());
    temp_file_name.resize(wcslen(temp_file_name.data()));
    //append file suffix
    temp_file_name+=file_suffix;
    //download file
    HINTERNET hInternet=InternetOpenW(L"ghost_installer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hInternet==NULL) {
        throw std::runtime_error("InternetOpenW failed");
    }
    HINTERNET hFile=InternetOpenUrlW(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if(hFile==NULL) {
        InternetCloseHandle(hInternet);
        throw std::runtime_error("InternetOpenUrlW failed");
    }
    HANDLE hFileHandle=CreateFileW(temp_file_name.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if(hFileHandle==INVALID_HANDLE_VALUE) {
        InternetCloseHandle(hFile);
        InternetCloseHandle(hInternet);
        throw std::runtime_error("CreateFileW failed");
    }
    DWORD dwBytesRead=0;
    DWORD dwBytesWritten=0;
    unsigned char buffer[1024];
    while(InternetReadFile(hFile, buffer, 1024, &dwBytesRead) && dwBytesRead>0) {
        WriteFile(hFileHandle, buffer, dwBytesRead, &dwBytesWritten, NULL);
    }
    CloseHandle(hFileHandle);
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return temp_file_name;
}
std::wstring get_ghost_url(){
    //TODO get ghost url from self file
}

int main() {
    if(IsSSPinstalled()) {
        try{
            auto nar_file=download_temp_file(get_ghost_url(), L".nar");
            auto SSPpath=GetSSPpath();
            //TODO install ghost
        }
    }
    else{
        //TODO download and install SSP
        //get language id
        //show install path dialog
        //install SSP(download zip & extract)
        //chose&install language pack & ghost for starter
        //install ghost
    }
    return 0;
}
