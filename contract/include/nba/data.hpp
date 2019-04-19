
#ifndef _INCLUDE_DATA_NBA_
#define _INCLUDE_DATA_NBA_

#include <map>
#include <string>
#include <tuple>
#include <eosio/eosio.hpp>

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

namespace json
{

// template <>
// struct json<nba::data>
// {
//     static nba::data from( const string &json ) {
//         return {
//             .game_id         = json::get_value<string>( json, "game_id" ),
//             .game_start_time = json::get_value<uint32_t>( json, "game_start_time" ),
//             .game_end_time   = json::get_value<uint32_t>( json, "game_end_time" ),
//             .home_team_id    = json::get_value<uint8_t>( json, "home_team_id" ),
//             .home_team_score = json::get_value<uint8_t>( json, "home_team_score" ),
//             .away_team_id    = json::get_value<uint8_t>( json, "away_team_id" ),
//             .away_team_score = json::get_value<uint8_t>( json, "away_team_score" )
//         };
//     }
// };

// template <> struct is_jsonable<nba::data> { static constexpr bool value = true; };

// template <>
// struct reflect<nba::data>
// {
//     typedef nba::data type;
//     static tuple<member<funtion<tuple<uint32_t, string>(type &)>, function<void(type &, uint32_t)>>,
//                  member<funtion<tuple<uint32_t, string>(type &)>, function<void(type &, uint32_t)>>>
//         instance() {
//             return {
//                 { [](type &i){return {i.lower_bound, "lower_bound"};}, [](type &i, uint32_t v){i.lower_bound = v;} },
//                 { [](type &i){return {i.upper_bound, "upper_bound"};}, [](type &i, uint32_t v){i.upper_bound = v;} }
//             };
//         }
// };

}

#endif
