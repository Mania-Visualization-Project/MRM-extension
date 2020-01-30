#include <windows.h>
#include <iostream>
#include <set>
#include <pthread.h>
#include <fstream>
#include <tchar.h>
#include "OsuManager.hpp"
#include "FileManager.hpp"
#include "ConfigManager.hpp"
#include "WindowsManager.hpp"

using namespace std;

const char *RENDER_NAME = "ManiaReplayMaster.jar";
const char *RENDER_PATH = "library\\ManiaReplayMaster.jar";

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

void *process(void *) {
    // 0. check if osu exists
    if (!Osu::get_osu_base_path()) {
        cerr << "Cannot find osu process. "
             << "Please open Osu!Mania rating interface before using this extension."
             << endl;
        return nullptr;
    }

    // 1. record replay files
    set<string> old_replays;
    get_replays(old_replays);

    // 2. send F2 key to export replay.osr
    Sleep(500); // wait user for releasing shortcut key
    WindowsManager::send_key(VK_F2);

    // 3. find if there is a new replay fetched (timeout: 20 seconds)
    char replay_file[1024] = {0};
    for (int i = 0; i < 20; i++) {
        Sleep(1000);
        if (find_replay(old_replays, replay_file)) {
            break;
        }
    }
    if (!*replay_file) {
        cerr << "Timeout: cannot find replay file!" << endl;
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
        cerr << "Cannot find beatmap file! "
             << "Please use ManiaReplayMaster.bat to input file names manually."
             << endl;
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
    WindowsManager::execute_cmd_blocking(bat_name);
    remove(bat_name);
    return nullptr;
}

void pause_exit(int code) {
    system("pause");
    exit(code);
}

int main(int argc, char *argv[]) {

    // check java
    char buf[BUF_SIZE];
    if (WindowsManager::execute_cmd("java -version 2>&1", buf) != 0) {
        cerr << "Cannot find Java. Please install Java 8 or latter version." << endl;
        pause_exit(-1);
    }

    // check render
    if (!FileManager::exist(RENDER_PATH)) {
        cerr << "Cannot find " << RENDER_PATH << ". Please copy this extension outside library dir." << endl;
        pause_exit(-2);
    }

    // check osu
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
        cerr << "Error in register hot key!" << endl;
        pause_exit(-3);
    }

    MSG msg = {nullptr};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            pthread_t pthread;
            pthread_create(&pthread, nullptr, process, nullptr);
        }
    }

}