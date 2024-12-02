#pragma once
#include "export_macros.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class LINUXAPI_DLL FileManagerAPI {
public:
    geode::Task<geode::Result<bool>> openDirectory(
        std::string location
    );
};