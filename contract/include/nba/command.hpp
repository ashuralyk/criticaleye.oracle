
#ifndef _INCLUDE_COMMAND_
#define _INCLUDE_COMMAND_

#include <vector>
#include <string>
#include <tuple>
#include <eosio/eosio.hpp>
#include "data.hpp"
#include "../util.hpp"

using namespace std;
using namespace eosio;

namespace nba
{

// fetch games in a period of time
struct period
{
    struct input
    {
        uint32_t lower_bound;
        uint32_t upper_bound;
    };

    struct output
    {
        vector<nba::data> periodic_games;
    };
};

// fetch one game specified by game's id
struct specified
{
    struct input
    {
        string game_id;
    };

    struct output
    {
        nba::data specified_game;
    };
};

}

namespace json
{

json_reflect_m( nba::period::input, (lower_bound), upper_bound )
json_reflect_s( nba::period::output, periodic_games )
json_reflect_s( nba::specified::input, game_id )
json_reflect_s( nba::specified::output, specified_game )

}

#endif
