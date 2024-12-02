#pragma once
#include "export_macros.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class LINUXAPI_DLL XDGAPI {
public:
    geode::Task<geode::Result<bool>> open(
        std::string path
    );
};