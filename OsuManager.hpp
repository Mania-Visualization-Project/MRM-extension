#ifndef _OSU_MANAGER_HPP_
#define _OSU_MANAGER_HPP_

#include <string>
#include <regex>

namespace Osu {
    char *get_osu_base_path();

    char *get_osu_replay_path();

    char *get_osu_songs_path();

    struct BeatmapHeader {
        std::string name;
        std::string artists;
        std::string difficulty;

        explicit BeatmapHeader(const std::string& replay_name);

        std::regex to_beatmap_regex();
    };
}



#endif //_OSU_MANAGER_HPP_