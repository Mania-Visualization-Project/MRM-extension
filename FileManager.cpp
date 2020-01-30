#include "FileManager.hpp"
#include <exception>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <direct.h>
#include <io.h>

namespace FileManager {

    void to_parent(char *file_name) {
        size_t index = strlen(file_name) - 1;
        for (; file_name[index] != '\\' && index >= 0; index--);
        if (index >= 0) {
            file_name[index] = '\0';
        } else {
            throw std::exception();
        }
    }

    bool walk_dir(const char *dir_name,
                  const std::function<bool(const char *, const char *)> &callback) {
        _finddata_t file{};
        long lf;
        char full_name[1024], dir_name_find[1024];

        strcpy(full_name, dir_name);
        strcat(full_name, "\\");
        size_t dir_name_len = strlen(full_name);

        strcpy(dir_name_find, dir_name);
        strcat(dir_name_find, "\\*");

        if ((lf = _findfirst(dir_name_find, &file)) == -1) {
            return true; // go on
        } else {
            while (_findnext(lf, &file) == 0) {
                if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                    continue;

                strcpy(full_name + dir_name_len, file.name);

                bool result;
                if (file.attrib & _A_SUBDIR) {
                    result = walk_dir(full_name, callback);
                } else {
                    result = callback(full_name, file.name);
                }
                if (!result) {
                    _findclose(lf);
                    return false;
                }
            }
        }
        _findclose(lf);
        return true;
    }

    void get_name(const char *full_name, char *buff) {
        size_t index = strlen(full_name) - 1;
        for (; full_name[index] != '\\' && index >= 0; index--);
        if (index >= 0) {
            strcpy(buff, full_name + index + 1);
        } else {
            throw std::exception();
        }
    }

    void create_folder(const char *name) {
        if (!exist(name)) {
            _mkdir(name);
        }
    }

    bool exist(const char *file_name) {
        return _access(file_name, F_OK) != -1;
    }
}