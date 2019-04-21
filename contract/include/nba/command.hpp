
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

#define nba_contract       "oracleosxnba"
#define nba_command_code_0 'n', 'b', 'a', '.', 'p', 'e', 'r', 'i', 'o', 'd', '.', 'i', 'n', 'p', 'u', 't', '.', 'v', '0'
#define nba_command_code_1 'n', 'b', 'a', '.', 'p', 'e', 'r', 'i', 'o', 'd', '.', 'i', 'n', 'p', 'u', 't', '.', 'v', '0'
#define nba_command_code_2 'n', 'b', 'a', '.', 's', 'p', 'e', 'c', 'i', 'f', 'i', 'e', 'd', '.', 'i', 'n', 'p', 'u', 't', '.', 'v', '0'
#define nba_command_code_3 'n', 'b', 'a', '.', 's', 'p', 'e', 'c', 'i', 'f', 'i', 'e', 'd', '.', 'o', 'u', 't', 'p', 'u', 't', '.', 'v', '0'

namespace nba
{

// fetch games in a period of time
namespace period
{
    struct input 
        : public util::command<name(nba_contract).value, nba_command_code_0>
    {
        uint32_t lower_bound;
        uint32_t upper_bound;
    };

    struct output
        : public util::command<name(nba_contract).value, nba_command_code_1>
    {
        vector<nba::data> periodic_games;
    };
}

// fetch one game specified by game's id
namespace specified
{
    struct input 
        : public util::command<name(nba_contract).value, nba_command_code_2>
    {
        string game_id;
    };

    struct output
        : public util::command<name(nba_contract).value, nba_command_code_3>
    {
        nba::data specified_game;
    };
}

}

#endif
