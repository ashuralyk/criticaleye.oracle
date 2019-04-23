
#ifndef _INCLUDE_COMMAND_
#define _INCLUDE_COMMAND_

#include <vector>
#include <string>
#include <tuple>
#include "data.hpp"
#include "../util.hpp"

using namespace std;
using namespace eosio;

struct nba_period_input
{
    uint32_t lower_bound;
    uint32_t upper_bound;
};

struct nba_period_output
{
    vector<nba::data> periodic_games;
};

struct nba_specified_input
{
    string game_id;
};

struct nba_specified_output
{
    nba::data specified_game;
};

#define nba_contract "oracleosxnba"
prototype( nba_period_input,     nba_contract, "nba.period.v1" )
prototype( nba_period_output,    nba_contract, "nba.period.v1" )
prototype( nba_specified_input,  nba_contract, "nba.specified.v1" )
prototype( nba_specified_output, nba_contract, "nba.specified.v1" )

#endif
