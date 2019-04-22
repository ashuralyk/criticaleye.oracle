
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
    if ( type == "period" )
    {
        criticaleye::require<nba::period::input>( get_self(), {
            .lower_bound = 0,
            .upper_bound = 1000
        });
    }
    else if ( type == "specified" )
    {
        criticaleye::require<nba::specified::input>( get_self(), {
            .game_id = "mm-xx-00001"
        });
    }
    else
    {
        util::rollback( "please choose 'period' or 'specified'" );
    }
}

void test::callback( nba::period::output &&output )
{
    print( "nba::period::output: " );
    for ( auto &v : output.periodic_games )
    {
        print( " > ", v.game_id );
    }
}

void test::callback( nba::specified::output &&output )
{
    print( "nba::specified::output: > ", output.specified_game.game_id );
}

EOSIO_DISPATCH( test, (pay)(receive) )
