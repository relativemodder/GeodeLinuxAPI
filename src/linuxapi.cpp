#include "wineutils.hpp"
#include <filesystem>
#include <linuxapi.hpp>


LinuxAPI* LinuxAPI::getInstance() {
    static LinuxAPI* instance;

    if (instance == nullptr) {
        instance = new LinuxAPI();
    }

    return instance;
}


std::string LinuxAPI::getPythonSidePath() {
    return std::filesystem::canonical(
        "geode/unzipped/relative.linuxapi/resources/relative.linuxapi"
    ).string();
}


void LinuxAPI::setupPythonScript() {
    auto home = WineUtils::getInstance()->getUnixHomeInWindows();

    auto originalServerPath = (std::filesystem::path(
        getPythonSidePath()
    ) / "linux-api-server.py").string();

	auto destinationServerPath = home + "\\linux-api-server.py";

	try {
		std::filesystem::copy(originalServerPath, destinationServerPath, std::filesystem::copy_options::overwrite_existing);
	}
	catch (const std::exception& e) {
		geode::log::error("Failed to copy server script: {}", e.what());
		return;
	}
}


void LinuxAPI::setupAutostartFile() {
    auto home = WineUtils::getInstance()->getUnixHomeInWindows();
    auto unix_home = WineUtils::getInstance()->getUnixHome();

    auto originalDesktopPath = (std::filesystem::path(
        getPythonSidePath()
    ) / "linux-api-server.desktop").string();

	auto destinationDesktopPath = home + "\\.config\\autostart\\linux-api-server.desktop";

	std::ifstream originalDesktopFile(originalDesktopPath);
	std::ofstream destinationDesktopFile(destinationDesktopPath);

	std::string line;
	while (std::getline(originalDesktopFile, line)) {
		size_t pos = line.find("<home>");
		if (pos != std::string::npos) {
			line.replace(pos, 6, unix_home);
		}
		destinationDesktopFile << line << "\n";
	}

	originalDesktopFile.close();
	destinationDesktopFile.close();
}


geode::Result<int> LinuxAPI::getServerPort() {
    auto tmp_port_path = WineUtils::getInstance()->unixPathToWindows("/tmp/linux-api-server-port");

    if (!std::filesystem::exists(tmp_port_path)) {
        return geode::Err("No server port file found");
    }

    std::ifstream port_file(tmp_port_path);
    if (!port_file.is_open()) {
        return geode::Err("Failed to open server port file");
    }

    std::string port_str;
    std::getline(port_file, port_str);
    port_file.close();

    try {
        return geode::Ok(std::stoi(port_str));
    } 
    catch (const std::invalid_argument& e) {
        return geode::Err("Invalid port number");
    } 
    catch (const std::out_of_range& e) {
        return geode::Err("Port number out of range");
    }
}


geode::Task<bool> LinuxAPI::isServerAlive() {
    return geode::Task<bool>::runWithCallback(
    [self = this] 
        (auto resolve, auto progress, auto cancelled) {
            auto port = self->getServerPort();

            if (port.ok() == false) {
                resolve(false);
                return;
            }

            geode::utils::web::WebRequest req = geode::utils::web::WebRequest();

            req.get(fmt::format("http://127.0.0.1:{}", port.unwrap())).listen(
                [resolve](geode::utils::web::WebResponse* response) {
                    if (response->ok() == false) {
                        resolve(false);
                        return;
                    }
                    resolve(true);
                }
            );
        }, 
    "Checking if server alive");
}


std::string ReplaceAll(std::string str, const std::string &from,
                       const std::string &to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos +=
        to.length(); // Handles case where 'to' is a substring of 'from'
  }
  return str;
}


std::string LinuxAPI::fixSpaces(std::string original) {
    return ReplaceAll(original, "%20", " ");
}