#pragma once
#include "export_macros.hpp"
#include <Windows.h>
#include <string>
#include <codecvt>
#include <locale>


class LINUXAPI_DLL WineUtils {
public:
    static WineUtils* getInstance();

public:
    bool isWine();
    std::string getWinePlatform();
    bool isLinux();
    std::string unixPathToWindows(std::string path);
    std::string windowsPathToUnix(std::string path);
    std::wstring s2ws(std::string str);
    std::string ws2s(std::wstring wstr);
    std::map<std::basic_string<TCHAR>, std::basic_string<TCHAR>> getEnvironment();
    std::string getUnixHome();
    std::string getUnixHomeInWindows();
};