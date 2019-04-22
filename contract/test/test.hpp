
#include "../include/criticaleye.hpp"

class [[eosio::contract("usefortest")]] test
    : public criticaleye
{
public:
    test( name self, name first_receiver, datastream<const char *> ds )
        : criticaleye( self, first_receiver, ds )
    {}

    [[eosio::action]]
    void pay( name server, checksum256 receipt, asset bill );

    [[eosio::action]]
    void receive( name server, checksum256 receipt, string type, vector<char> data );

    [[eosio::action]]
    void require( string type );

    void callback( nba::period::output &&output ) override;

    void callback( nba::specified::output &&output ) override;
};
