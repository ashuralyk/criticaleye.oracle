
#include "test.hpp"

void test::pay( name server, checksum256 receipt, asset bill )
{
    gateway::pay( server, receipt, bill );
}

void test::receive( name server, checksum256 receipt, string type, vector<char> data )
{
    gateway::receive( server, receipt, type, data );
}

void test::callback( nba::period::output &&output )
{

}

void test::callback( nba::specified::output &&output )
{

}

EOSIO_DISPATCH( test, (pay)(receive) )
