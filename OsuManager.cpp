#include <cstdio>
#include <cstring>
#include <iostream>
#include "OsuManager.hpp"
#include "FileManager.hpp"

#define IS_LINE_END(c) ((c) == '\r' || (c) == '\n')
#define BUF_SIZE 1024

namespace Osu {

    static char osu_path[BUF_SIZE] = {0};
    static char osu_replay_path[BUF_SIZE] = {0};
    static char osu_songs_path[BUF_SIZE] = {0};

    static void execute_cmd(const char *cmd, char *result) {
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
            pclose(ptr);
        } else {
            std::cout << "Error in: " << cmd << std::endl;
        }
    }

    static void update_osu_path(const char *base_dir) {
        strcpy(osu_path, base_dir);

        strcpy(osu_replay_path, base_dir);
        strcat(osu_replay_path, "\\Replays");

        strcpy(osu_songs_path, base_dir);
        strcat(osu_songs_path, "\\Songs");
    }

    static char *read_line(char *start, char *buf) {
        while (!IS_LINE_END(*start)) {
            if (buf) {
                *buf = *start;
                buf++;
            }
            start++;
        }
        if (buf) *buf = '\0';
        return start + 1;
    }

    char *get_osu_base_path() {
        if (*osu_path != '\0') {
            return osu_path;
        }
        char buf[BUF_SIZE] = {0};
        char path_buf[BUF_SIZE] = {0};
        execute_cmd("powershell \"get-process osu! | select-object path\"", buf);
        /**
         * If process osu! exists, it will print "\nPath\n----[path]"
         * Otherwise, it will print "get-process :..."
         */
        if (IS_LINE_END(buf[0])) {
            char *pbuf = buf;
            pbuf = read_line(pbuf, nullptr);
            pbuf = read_line(pbuf, nullptr);
            pbuf = read_line(pbuf, nullptr);
            read_line(pbuf, path_buf);
            FileManager::to_parent(path_buf);

            update_osu_path(path_buf);

            return osu_path;
        } else {
            return nullptr;
        }
    }

    char *get_osu_replay_path() {
        get_osu_base_path(); // ensure updated
        return osu_replay_path;
    }

    char *get_osu_songs_path() {
        get_osu_base_path(); // ensure updated
        return osu_songs_path;
    }

    BeatmapHeader::BeatmapHeader(const std::string &replay_name) {
        // replay format: player - artists - name - [difficulty] (date) OsuMania-n.osr
        char base_name[1024];
        FileManager::get_name(replay_name.c_str(), base_name);
        int pos1 = replay_name.find(" - ");
        int pos2 = replay_name.find(" - ", pos1 + 1);
        int pos3 = replay_name.find(" [", pos2 + 1);
        int pos4 = replay_name.rfind("] ");
        artists = replay_name.substr(pos1 + 3, pos2 - pos1 - 3);
        name = replay_name.substr(pos2 + 3, pos3 - pos2 - 3);
        difficulty = replay_name.substr(pos3 + 2, pos4 - pos3 - 2);
    }

    static void replace_all(
            std::string &str,
            const std::string &old_value,
            const std::string &new_value
    ) {
        std::string::size_type pos(0);
        while (true) {
            if ((pos = str.find(old_value, pos)) != std::string::npos) {
                str = str.replace(pos, old_value.length(), new_value);
                pos = pos + new_value.size();
            }
            else break;
        }
    }

    static void escape(std::string &str) {
        std::string escapes[] = {"\\", "$", "(", ")", "*", "+", ".", "[", "]", "?", "^", "{", "}",
                                "|"};
        for (const auto& s : escapes) {
            replace_all(str, s, "\\" + s);
        }
        replace_all(str, " ", "\\s");
    }

    std::regex BeatmapHeader::to_beatmap_regex() {
        // beatmap file format: artists - name (mapper) [difficulty].osu
        std::stringstream pattern_stream;
        escape(name);
        escape(artists);
        escape(difficulty);
        pattern_stream << "^" << artists
                       << "\\s-\\s"
                       << name
                       << "\\s"
                       << "\\(.*\\)" // mapper
                       << "\\s"
                       << "\\["
                       << difficulty
                       << "\\]\\.osu$";
        auto pattern = pattern_stream.str();
//        std::cout << "Generate beatmap name regex pattern: " << pattern << std::endl;
        return std::regex(pattern);
    }
}