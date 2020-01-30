#ifndef _FILE_MANAGER_HPP_
#define _FILE_MANAGER_HPP_

#include <cstring>
#include <functional>

namespace FileManager {
    void to_parent(char *file_name);

    /**
     * walk a dir recursively
     * @param dir_name
     * @param callback : callback(fullname, filename) -> if need continue
     * @return if need continue (use in recursion)
     */
    bool walk_dir(
            const char *dir_name,
            const std::function<bool(const char *, const char *)> &callback);

    void get_name(const char *full_name, char *buff);

    bool exist(const char *file_name);

    void create_folder(const char *name);
}

#endif