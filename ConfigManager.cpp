#include "ConfigManager.hpp"
#include <windows.h>
#include <iostream>

#define APP_NAME "MRM"

std::string ConfigManager::get(const char *key) {
    char buff[CONFIG_BUFF_SIZE];
    GetPrivateProfileString(
            APP_NAME,
            key,
            "",
            buff,
            CONFIG_BUFF_SIZE,
            CONFIG_FILE_NAME
    );
    return std::string(buff);
}

void ConfigManager::put(const char *key, const char *value) {
    WritePrivateProfileString(
            APP_NAME,
            key,
            value,
            CONFIG_FILE_NAME
    );
}

void ConfigManager::initialize_config() {
    if (!FileManager::exist(CONFIG_FILE_NAME)) {
        for (auto &item : ConfigItems) {
            put(item.key, item.default_value);
        }
    }
}

ConfigManager::ConfigItem::ConfigItem(const char *key, const char *defaultValue) {
    strcpy(this->key, key);
    strcpy(this->default_value, defaultValue);
}
