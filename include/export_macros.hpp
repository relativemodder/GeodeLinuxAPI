#pragma once

#ifdef GEODE_IS_WINDOWS
    #ifdef RELATIVE_LINUXAPI_EXPORTING
        #define LINUXAPI_DLL __declspec(dllexport)
    #else
        #define LINUXAPI_DLL __declspec(dllimport)
    #endif
#else
    #define LINUXAPI_DLL __attribute__((visibility("default")))
#endif