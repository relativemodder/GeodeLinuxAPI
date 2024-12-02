#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class FileManagerAPI {
public:
    geode::Task<geode::Result<bool>> openDirectory(
        std::string location
    );
};