
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
struct period : public util::command<0>
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
struct specified : public util::command<1>
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

inline void check_require( uint8_t data_type, vector<char> &data )
{
    switch ( data_type )
    {
        case period::type:    unpack<period::input>( data ); return;
        case specified::type: unpack<specified::input>( data ); return;
    }
    internal_use_do_not_use::eosio_assert( false, "invalid data_type" );
}

inline void check_response( uint8_t data_type, vector<char> &data )
{
    switch ( data_type )
    {
        case period::type:    unpack<period::output>( data ); return;
        case specified::type: unpack<specified::output>( data ); return;
    }
    internal_use_do_not_use::eosio_assert( false, "invalid data_type" );
}

}

#endif
