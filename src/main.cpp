#include "Geode/binding/FLAlertLayer.hpp"
#include <Geode/Geode.hpp>
#include <wineutils.hpp>
#include <linuxapi.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;


class $modify(LinuxAPIMenuLayer, MenuLayer) {
    void checkTask() {
		geode::log::info("Checking for server running");

		LinuxAPI::getInstance()->isServerAlive()
		.listen([self = this] (bool* result) {  
			bool res = *result;

			if (res) return;

			geode::createQuickPopup(
				"Linux API", "Looks like your Python Side <cr>is not running.</c> You might relogin into your desktop environment or start it manually.", 
				"OK", nullptr, 
				nullptr
			);
		});
	}

    bool init() {
		if (!MenuLayer::init()) return false;

		auto call = CCCallFunc::create(
            this, 
            callfunc_selector(LinuxAPIMenuLayer::checkTask)
        );
		runAction(call);

		return true;
	}
};


$execute {
    if (!WineUtils::getInstance()->isLinux()) {
        Notification::create(
            "Linux API: your platform is not supported", 
            NotificationIcon::Error
        )->show();
		return;
    }

    
    auto linuxAPI = LinuxAPI::getInstance();

    linuxAPI->setupPythonScript();
    linuxAPI->setupAutostartFile();
}