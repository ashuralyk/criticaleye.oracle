
#ifndef _INCLUDE_COMMAND_
#define _INCLUDE_COMMAND_

#include <vector>
#include <string>
#include <eosio/eosio.hpp>

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
        
        string to_json() {
            return "{type:\"period\",lower_bound:" + to_string(lower_bound) + ",upper_bound:" + to_string(upper_bound) + "}";
        }
    };

    template <class _game_data>
    struct output
    {
        vector<_game_data> periodic_games;
    };
};

// fetch one game specified by game's id
struct specified
{
    struct input
    {
        string game_id;

        string to_json() {
            return "{type:\"specified\",game_id:" + game_id + "}";
        }
    };

    template <class _game_data>
    struct output
    {
        _game_data specified_game;
    };
};

}

#endif
