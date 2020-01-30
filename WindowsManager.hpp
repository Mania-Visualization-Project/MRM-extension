#ifndef _WINDOWS_MANAGER_HPP_
#define _WINDOWS_MANAGER_HPP_

#include <windows.h>

#define BUF_SIZE 1024

namespace WindowsManager {
    int execute_cmd(const char *cmd, char *result);

    void execute_cmd_blocking(const char *command);

    void send_key(char key);
}
#endif //_WINDOWS_MANAGER_HPP_
