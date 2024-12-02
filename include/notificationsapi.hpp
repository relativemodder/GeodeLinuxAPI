#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>


class NotificationsAPI {
public:
    geode::Task<geode::Result<bool>> addNotification(
        std::string title,
        std::string body
    );
};