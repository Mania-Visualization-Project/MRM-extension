#ifndef _CONFIGMANAGER_HPP_
#define _CONFIGMANAGER_HPP_

#include "FileManager.hpp"

#define CONFIG_FILE_NAME ".\\configuration.ini"
#define CONFIG_BUFF_SIZE 1024

#define KEY_SPEED "speed"
#define DEFAULT_SPEED "15"

namespace ConfigManager {

    struct ConfigItem {
        char key[256]{};
        char default_value[256]{};

        ConfigItem(const char *key, const char *defaultValue);
    };

    static ConfigItem ConfigItems[] = {{KEY_SPEED, DEFAULT_SPEED}};

    std::string get(const char *key);

    void put(const char *key, const char *value);

    void initialize_config();


}
#endif //_CONFIGMANAGER_HPP_
