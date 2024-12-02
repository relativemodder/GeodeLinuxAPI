#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class XDGAPI {
public:
    geode::Task<geode::Result<bool>> open(
        std::string path
    );
};