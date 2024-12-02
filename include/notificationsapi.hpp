#pragma once
#include "export_macros.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class LINUXAPI_DLL NotificationsAPI {
public:
    geode::Task<geode::Result<bool>> addNotification(
        std::string title,
        std::string body
    );
};