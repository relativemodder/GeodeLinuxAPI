#include <Geode/Geode.hpp>
#include "wineutils.hpp"
#include "linuxapi.hpp"
#include <Geode/modify/MenuLayer.hpp>
#include <filesystem>

using namespace geode::prelude;


class $modify(LinuxAPIMenuLayer, MenuLayer) {
    void checkTask() {
		geode::log::info("Checking for server running");

		LinuxAPI::getInstance()->isServerAlive()
		.listen([self = this] (bool* result) {  
			bool res = *result;

			if (res) return;

			auto wineUtils = WineUtils::getInstance();

			if (std::filesystem::exists(wineUtils->unixPathToWindows("/app"))) {
				log::info("This bastard is using fucking flatpak");
				geode::createQuickPopup(
					"Linux API", 
					fmt::format(
						"{} {} {}\n\n{}", 
						"<cl>Flatpak</c> detected. Consider allowing <cr>flatpak client</c>",
						"(Steam or whatever you use) to access your <cy>home directory.</c>",
						"Then, <co>restart</c> the flatpak client to apply changes.",
						"(You can do it by using <cg>Flatseal</c> btw)"
					), 
					"OK", nullptr, 
					nullptr
				);
				return;
			}

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
		return;
    }

    auto linuxAPI = LinuxAPI::getInstance();

    linuxAPI->setupPythonScript();
    linuxAPI->setupAutostartFile();
}