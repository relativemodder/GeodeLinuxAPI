#pragma once
#include "export_macros.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class LINUXAPI_DLL FilePickerAPI {
public:
    geode::Task<geode::Result<std::vector<std::string>>> openFiles(
        bool multipleFiles = false,
        bool openDirectoryMode = false
    );
    geode::Task<geode::Result<std::string>> saveFile(
        std::string filename = ""
    );
};