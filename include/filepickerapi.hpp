#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class FilePickerAPI {
public:
    geode::Task<geode::Result<std::vector<std::string>>> openFiles(
        bool multipleFiles = false,
        bool openDirectoryMode = false
    );
    geode::Task<geode::Result<std::string>> saveFile(
        std::string filename = ""
    );
};