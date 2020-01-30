#include <windows.h>
#include <iostream>
#include <set>
#include <pthread.h>
#include <fstream>
#include <tchar.h>
#include "OsuManager.hpp"
#include "FileManager.hpp"
#include "ConfigManager.hpp"

using namespace std;

const char *RENDER_NAME = "ManiaReplayMaster.jar";
const char *RENDER_PATH = "library\\ManiaReplayMaster.jar";

void send_key(char key) {
    int scanCode = MapVirtualKey(key, 0);
    keybd_event(key, scanCode, 0, 0);
    Sleep(100);
    keybd_event(key, scanCode, KEYEVENTF_KEYUP, 0);
}

void get_replays(set<string> &set) {
    FileManager::walk_dir(Osu::get_osu_replay_path(),
                          [&](const char *full_name, const char *file_name) {
                              set.emplace(file_name);
                              return true;
                          });
}

bool find_replay(set<string> &old_replays, char *replay_name) {
    FileManager::walk_dir(Osu::get_osu_replay_path(),
                          [&](const char *full_name, const char *file_name) -> bool {
                              string current_file(file_name);
                              bool in = false;
                              for (auto &old_file : old_replays) {
                                  if (old_file == current_file) {
                                      in = true;
                                      break;
                                  }
                              }
                              if (!in) {
                                  strcpy(replay_name, full_name);
                                  return false;
                              }
                              return true;
                          });
    return *replay_name != '\0';
}

bool find_beatmap(const std::regex &regex, char *beatmap_name) {
    FileManager::walk_dir(Osu::get_osu_songs_path(),
                          [&](const char *full_name, const char *file_name) -> bool {
                              std::smatch match;
                              std::string file(file_name);
                              if (std::regex_match(file, match, regex)) {
                                  strcpy(beatmap_name, full_name);
                                  return false;
                              }
                              return true;
                          });
    return *beatmap_name != '\0';
}

void shell_execute_blocking(const char *command) {
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

void *process(void *) {
    // 0. check if osu exists
    if (!Osu::get_osu_base_path()) {
        cout << "Cannot find osu process. "
             << "Please open Osu!Mania rating interface before using this extension."
             << endl;
        return nullptr;
    }

    // 1. record replay files
    set<string> old_replays;
    get_replays(old_replays);

    // 2. send F2 key to export replay.osr
    Sleep(500); // wait user for releasing shortcut key
    send_key(VK_F2);

    // 3. find if there is a new replay fetched (timeout: 20 seconds)
    char replay_file[1024] = {0};
    for (int i = 0; i < 20; i++) {
        Sleep(1000);
        if (find_replay(old_replays, replay_file)) {
            break;
        }
    }
    if (!*replay_file) {
        cout << "Timeout: cannot find replay file!" << endl;
        return nullptr;
    }
    cout << "Find replay: " << replay_file << endl;

    // 4. search for beatmap according to replay file name
    Osu::BeatmapHeader beatmap(replay_file);
    auto regex = beatmap.to_beatmap_regex();
    char beatmap_file[1024] = {0};
    if (find_beatmap(regex, beatmap_file)) {
        cout << "Find beatmap: " << beatmap_file << endl;
    } else {
        cout << "Cannot find beatmap file!" << endl;
        return nullptr;
    }

    // 5. invoke render!
    char bat_name[1024] = ".\\batch\\";
    FileManager::create_folder("batch");
    FileManager::get_name(beatmap_file, bat_name + strlen(bat_name));
    strcpy(bat_name + strlen(bat_name) - 3, "bat");
    stringstream command;
    command << "@echo off\ncd .\\library\njava -jar " << RENDER_NAME
            << " -speed=" << ConfigManager::get(KEY_SPEED)
            << " \"" << beatmap_file << "\""
            << " \"" << replay_file << "\""
            << "\npause";
    ofstream batch(bat_name);
    batch << command.str();
    batch.close();
    shell_execute_blocking(bat_name);
    remove(bat_name);
    return nullptr;
}

int main(int argc, char *argv[]) {

    if (!FileManager::exist(RENDER_PATH)) {
        cout << "Cannot find " << RENDER_PATH << ". Please copy this extension outside library.";
        return -1;
    }

    auto base_path = Osu::get_osu_base_path();
    if (base_path) {
        cout << "Find osu base path: " << base_path << endl;
    }

    ConfigManager::initialize_config();
    cout << "Use speed = " << ConfigManager::get(KEY_SPEED) << ". You can change this speed in "
         << CONFIG_FILE_NAME << "." << endl;

    if (RegisterHotKey(nullptr, 1, 0, VK_F1)) {
        cout << "Press F1 in Osu!Mania rating interface." << endl;
    } else {
        cout << "Error in register hot key!" << endl;
        return -1;
    }

    MSG msg = {nullptr};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            pthread_t pthread;
            pthread_create(&pthread, nullptr, process, nullptr);
        }
    }

}