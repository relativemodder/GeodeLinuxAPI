#include <xdgapi.hpp>

#include "wineutils.hpp"
#include <linuxapi.hpp>
#include <matjson.hpp>


geode::Task<geode::Result<bool>> XDGAPI::open(
    std::string path
) {
    return geode::Task<geode::Result<bool>>::runWithCallback(
        [path]
        (auto resolve, auto progress, auto cancelled) {
            geode::log::info("Checking if server is alive");
            auto alive = LinuxAPI::getInstance()->isServerAlive();
            alive.listen(
                [
                    resolve, 
                    path
                ] (bool* aliveResultPtr) {
                    bool aliveResult = *aliveResultPtr;

                    geode::log::info("Alive result: {}", aliveResult);

                    if (!aliveResult) {
                        resolve(geode::Err("Linux API server is dead"));
                        return;
                    }

                    auto port = LinuxAPI::getInstance()->getServerPort().unwrap();

                    geode::utils::web::WebRequest req = geode::utils::web::WebRequest();
                    
                    geode::log::info("Making request to /xdg/open");

                    auto reqPayload = matjson::makeObject({
                        {"path", path}
                    });

                    req.bodyJSON(reqPayload);

                    req.post(fmt::format("http://127.0.0.1:{}/xdg/open", port)).listen(
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

                            resolve(geode::Ok(true));                       
                        }
                    );
                }
            );
        }
    );
}