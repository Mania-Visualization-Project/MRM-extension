#include <tchar.h>
#include <cstdio>
#include <iostream>
#include "WindowsManager.hpp"

namespace WindowsManager {

    int execute_cmd(const char *cmd, char *result) {
        char buf_ps[BUF_SIZE];
        char ps[BUF_SIZE] = {0};
        FILE *ptr;
        strcpy(ps, cmd);
        if ((ptr = popen(ps, "r")) != nullptr) {
            while (fgets(buf_ps, BUF_SIZE, ptr) != nullptr) {
                strcat(result, buf_ps);
                if (strlen(result) > BUF_SIZE)
                    break;
            }
            return pclose(ptr);
        } else {
            std::cerr << "Error in: " << cmd << std::endl;
            return -1;
        }
    }

    void execute_cmd_blocking(const char *command) {
        SHELLEXECUTEINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SEE_MASK_NOCLOSEPROCESS;
        si.lpVerb = _T("open");
        si.lpFile = _T(command);
        si.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&si);

        DWORD dwExitCode;
        GetExitCodeProcess(si.hProcess, &dwExitCode);
        while (dwExitCode == STILL_ACTIVE) {
            Sleep((DWORD) 200);
            GetExitCodeProcess(si.hProcess, &dwExitCode);
        }

        CloseHandle(si.hProcess);
    }

    void send_key(char key) {
        int scanCode = MapVirtualKey(key, 0);
        keybd_event(key, scanCode, 0, 0);
        Sleep(100);
        keybd_event(key, scanCode, KEYEVENTF_KEYUP, 0);
    }
}