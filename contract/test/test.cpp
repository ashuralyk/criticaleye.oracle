
#include "test.hpp"

void test::require( string type )
{
    require_auth( get_self() );

    if ( type == "period" )
    {
        auto receipt = criticaleye::require<nba::period_input>( get_self(), {
            .lower_bound = 0,
            .upper_bound = 1000
        });
        print( "receipt(period) = ", receipt );
    }
    else if ( type == "specified" )
    {
        auto receipt = criticaleye::require<nba::specified_input>( get_self(), {
            .game_id = "mm-xx-00001"
        });
        print( "receipt(specified) = ", receipt );
    }
    else
    {
        util::rollback( "please choose 'period' or 'specified'" );
    }
}

void test::callback( nba::period_output &&output )
{
    response_index.emplace( get_self(), [&](auto &v) {
        v.time = current_time_point().time_since_epoch().count();
        v.period_output = output;
    });

    print( "nba::period_output: " );
    for ( auto &v : output.periodic_games )
    {
        print( " > ", v.game_id );
    }
}

void test::callback( nba::specified_output &&output )
{
    response_index.emplace( get_self(), [&](auto &v) {
        v.time = current_time_point().time_since_epoch().count();
        v.specified_output = output;
    });

    print( "nba_specified_output: > ", output.specified_game.game_id );
}

EOSIO_DISPATCH( test, (pay)(receive)(require) )
