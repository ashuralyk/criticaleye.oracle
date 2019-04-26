
#ifndef _INCLUDE_COMMAND_
#define _INCLUDE_COMMAND_

#include <vector>
#include <string>
#include <tuple>
#include "data.hpp"
#include "../util.hpp"

using namespace std;
using namespace eosio;

namespace nba
{

struct period_input
{
    uint32_t lower_bound;
    uint32_t upper_bound;
};

struct period_output
{
    vector<nba::data> periodic_games;
};

struct specified_input
{
    string game_id;
};

struct specified_output
{
    nba::data specified_game;
};

}

prototype( nba::period_input,     nba_contract, "nba.period.v1" )
prototype( nba::period_output,    nba_contract, "nba.period.v1" )
prototype( nba::specified_input,  nba_contract, "nba.specified.v1" )
prototype( nba::specified_output, nba_contract, "nba.specified.v1" )

#endif
