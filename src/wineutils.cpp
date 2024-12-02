#include <wineutils.hpp>


WineUtils* WineUtils::getInstance() {
    static WineUtils* instance;
    if (instance == nullptr) {
        instance = new WineUtils();
    }

    return instance;
}


bool WineUtils::isWine() {
    HMODULE hntdll = GetModuleHandle("ntdll.dll");
    auto proc_exists = (void *)GetProcAddress(hntdll, "wine_get_version");

    return proc_exists != nullptr;
}


std::string WineUtils::getWinePlatform() {
    static void (CDECL *pwine_get_host_version)(char **sysname, char **release);
    HMODULE hntdll = GetModuleHandle("ntdll.dll");
    pwine_get_host_version = (void (CDECL *)(char **, char **))GetProcAddress(hntdll, "wine_get_host_version");

    char* sysname;
    char* version;
    pwine_get_host_version(&sysname, &version);

    return std::string(sysname);
}


bool WineUtils::isLinux() {
    if (!isWine()) return false;

    return getWinePlatform() == "Linux";
}


std::wstring WineUtils::s2ws(std::string str) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}


std::string ws2s(std::wstring wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}


std::string WineUtils::unixPathToWindows(std::string path) {
    LPWSTR (*CDECL wine_get_dos_file_name_ptr)(LPCSTR) = NULL;

    wine_get_dos_file_name_ptr = (LPWSTR (*)(LPCSTR))GetProcAddress(
        GetModuleHandleA("KERNEL32"),
        "wine_get_dos_file_name"
    );

    auto rawResult = wine_get_dos_file_name_ptr(path.c_str());
    auto wresult = std::wstring(rawResult);

    auto result = std::string(wresult.begin(), wresult.end());
    
    return result;
}


std::string WineUtils::windowsPathToUnix(std::string path) {
    LPSTR (*CDECL wine_get_unix_file_name_ptr)(LPCWSTR) = NULL;

    wine_get_unix_file_name_ptr = (LPSTR (*)(LPCWSTR))GetProcAddress(
        GetModuleHandleA("KERNEL32"),
        "wine_get_unix_file_name"
    );

    auto rawResult = wine_get_unix_file_name_ptr(s2ws(path).c_str());
    auto result = std::string(rawResult);

    return result;
}


std::map<std::basic_string<TCHAR>, std::basic_string<TCHAR>> WineUtils::getEnvironment() {
    typedef std::basic_string<TCHAR> tstring; // Generally convenient
    std::map<tstring, tstring> env;

    auto free = [](LPTCH p) { FreeEnvironmentStrings(p); };
    auto env_block = std::unique_ptr<TCHAR, decltype(free)>{
            GetEnvironmentStrings(), free};

    for (LPTCH i = env_block.get(); *i != TCHAR('\0'); ++i) {
        tstring key;
        tstring value;

        for (; *i != TCHAR('='); ++i)
            key += *i;
        ++i;
        for (; *i != TCHAR('\0'); ++i)
            value += *i;

        env[key] = value;
    }

    return env;
}


std::string WineUtils::getUnixHome() {
    auto envs = getEnvironment();
	auto wine_username = envs["WINEUSERNAME"];

	geode::log::info("Wine username: {}", wine_username);

    return "/home/" + wine_username;
}

std::string WineUtils::getUnixHomeInWindows() {
  return unixPathToWindows(getUnixHome());
}