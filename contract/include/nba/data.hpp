
#ifndef _INCLUDE_DATA_NBA_
#define _INCLUDE_DATA_NBA_

#include <map>
#include <string>
#include <tuple>

using namespace std;

namespace nba
{

inline tuple<string, string, string> get_team_by_id(uint8_t team_id)
{
    struct team
    {
        string short_name;
        string full_name_chs;
        string full_name_en;
    };

    map<uint8_t, team> id_to_team = {
        { 0, {"", "", ""} },
        { 1, {"", "", ""} },
        { 2, {"", "", ""} },
        { 3, {"", "", ""} },
        { 4, {"", "", ""} },
    };

    auto &nba = id_to_team[team_id];
    return { nba.short_name, nba.full_name_chs, nba.full_name_en };
}

struct data
{
    string   game_id;
    uint32_t game_start_time;
    uint32_t game_end_time;
    uint8_t  home_team_id;
    uint8_t  home_team_score;
    uint8_t  away_team_id;
    uint8_t  away_team_score;
};

}

#endif
