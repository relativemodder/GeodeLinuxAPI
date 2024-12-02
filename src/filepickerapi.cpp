#include "wineutils.hpp"
#include "filepickerapi.hpp"
#include "linuxapi.hpp"
#include <matjson.hpp>


geode::Task<geode::Result<std::vector<std::string>>> FilePickerAPI::openFiles(
    bool multipleFiles,
    bool openDirectoryMode
) {
    return geode::Task<geode::Result<std::vector<std::string>>>::runWithCallback(
        [multipleFiles, openDirectoryMode]
        (auto resolve, auto progress, auto cancelled) {
            geode::log::info("Checking if server is alive");
            auto alive = LinuxAPI::getInstance()->isServerAlive();
            alive.listen(
                [
                    resolve, 
                    multipleFiles, 
                    openDirectoryMode
                ] (bool* aliveResultPtr) {
                    bool aliveResult = *aliveResultPtr;

                    if (!aliveResult) {
                        resolve(geode::Err("Linux API server is dead"));
                        return;
                    }

                    auto port = LinuxAPI::getInstance()->getServerPort().unwrap();

                    geode::utils::web::WebRequest req = geode::utils::web::WebRequest();
                    
                    geode::log::info("Making request to /files/open");

                    auto reqPayload = matjson::makeObject({
                        {"multiple_files", multipleFiles},
                        {"open_directory", openDirectoryMode}
                    });

                    req.bodyJSON(reqPayload);

                    req.post(fmt::format("http://127.0.0.1:{}/files/open", port)).listen(
                        [resolve](geode::utils::web::WebResponse* response) {
                            if (response->ok() == false) {
                                geode::log::error("Trying to unwrap an error");
                                resolve(geode::Err(response->string().unwrapErr()));
                                return;
                            }

                            auto parsed = matjson::parse(response->string().unwrap());

                            if (parsed.ok() == false) {
                                geode::log::error("Error parsing JSON");
                                resolve(geode::Err("Error parsing JSON"));
                                return;
                            }

                            if (parsed.unwrap().contains("error")) {
                                geode::log::error("Error from server: {}", parsed.unwrap()["error"].asString().unwrap());
                                resolve(
                                    geode::Err(
                                        parsed.unwrap()["error"].asString().unwrap()
                                    )
                                );
                                return;
                            }

                            if (!parsed.unwrap().contains("files")) {
                                resolve(
                                    geode::Err(
                                        response->string().unwrap()
                                    )
                                );
                                return;
                            }

                            auto linuxAPI = LinuxAPI::getInstance();

                            auto files = parsed.unwrap()["files"].as<std::vector<std::string>>().unwrap();
                            auto finalFiles = std::vector<std::string>();

                            for (auto rawFile : files) {
                                auto winPath = WineUtils::getInstance()->unixPathToWindows(
                                    linuxAPI->fixSpaces(rawFile)
                                );
                                finalFiles.push_back(
                                    winPath
                                );

                                geode::log::info("Raw {} path converted to {}", rawFile, winPath);
                            }
                            
                            resolve(
                                geode::Ok(
                                    finalFiles
                                )
                            );
                            
                        }
                    );
                }
            );
        }
    );
}


geode::Task<geode::Result<std::string>> FilePickerAPI::saveFile(
    std::string filename
) {
    return geode::Task<geode::Result<std::string>>::runWithCallback(
        [filename]
        (auto resolve, auto progress, auto cancelled) {
            geode::log::info("Checking if server is alive");
            auto alive = LinuxAPI::getInstance()->isServerAlive();
            alive.listen(
                [
                    resolve,
                    filename
                ] (bool* aliveResultPtr) {
                    bool aliveResult = *aliveResultPtr;

                    if (!aliveResult) {
                        resolve(geode::Err("Linux API server is dead"));
                        return;
                    }

                    auto port = LinuxAPI::getInstance()->getServerPort().unwrap();

                    geode::utils::web::WebRequest req = geode::utils::web::WebRequest();
                    
                    geode::log::info("Making request to /files/save");

                    auto reqPayload = matjson::makeObject({
                        {"file_name", filename}
                    });

                    req.bodyJSON(reqPayload);

                    req.post(fmt::format("http://127.0.0.1:{}/files/save", port)).listen(
                        [resolve](geode::utils::web::WebResponse* response) {
                            if (response->ok() == false) {
                                geode::log::error("Trying to unwrap an error");
                                resolve(geode::Err(response->string().unwrapErr()));
                                return;
                            }

                            auto parsed = matjson::parse(response->string().unwrap());

                            if (parsed.ok() == false) {
                                geode::log::error("Error parsing JSON");
                                resolve(geode::Err("Error parsing JSON"));
                                return;
                            }

                            if (parsed.unwrap().contains("error")) {
                                geode::log::error("Error from server: {}", parsed.unwrap()["error"].asString().unwrap());
                                resolve(
                                    geode::Err(
                                        parsed.unwrap()["error"].asString().unwrap()
                                    )
                                );
                                return;
                            }

                            if (!parsed.unwrap().contains("file")) {
                                resolve(
                                    geode::Err(
                                        response->string().unwrap()
                                    )
                                );
                                return;
                            }

                            auto file = parsed.unwrap()["file"].asString().unwrap();

                            resolve(geode::Ok(
                                WineUtils::getInstance()->unixPathToWindows(
                                    LinuxAPI::getInstance()->fixSpaces(
                                        file
                                    )
                                )
                            ));
                            
                        }
                    );
                }
            );
        }
    );
}