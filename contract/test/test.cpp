
#include "test.hpp"

void test::pay( name server, checksum256 receipt, asset bill )
{
    criticaleye::pay( server, receipt, bill );
}

void test::receive( name server, checksum256 receipt, string type, vector<char> data )
{
    criticaleye::receive( server, receipt, type, data );
}

void test::require( string type )
{
    require_auth( get_self() );

    if ( type == "period" )
    {
        auto receipt = criticaleye::require<nba_period_input>( get_self(), {
            .lower_bound = 0,
            .upper_bound = 1000
        });
        print( "receipt(period) = ", receipt );
    }
    else if ( type == "specified" )
    {
        auto receipt = criticaleye::require<nba_specified_input>( get_self(), {
            .game_id = "mm-xx-00001"
        });
        print( "receipt(specified) = ", receipt );
    }
    else
    {
        util::rollback( "please choose 'period' or 'specified'" );
    }
}

void test::callback( nba_period_output &&output )
{
    print( "nba_period_output: " );
    for ( auto &v : output.periodic_games )
    {
        print( " > ", v.game_id );
    }
}

void test::callback( nba_specified_output &&output )
{
    print( "nba_specified_output: > ", output.specified_game.game_id );
}

EOSIO_DISPATCH( test, (pay)(receive)(require) )
