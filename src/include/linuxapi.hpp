#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <filesystem>
#include <string>
#include <wineutils.hpp>
#include <filepickerapi.hpp>
#include <filemanagerapi.hpp>
#include <notificationsapi.hpp>
#include <xdgapi.hpp>


class LinuxAPI : public FilePickerAPI, 
    public FileManagerAPI, 
    public NotificationsAPI,
    public XDGAPI {
public:
    static LinuxAPI* getInstance();

public:
    std::string getPythonSidePath();
    void setupPythonScript();
    void setupAutostartFile();
    geode::Result<int> getServerPort();
    geode::Task<bool> isServerAlive();
    std::string fixSpaces(std::string original);
};