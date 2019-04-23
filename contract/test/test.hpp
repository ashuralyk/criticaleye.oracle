
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

    void callback( nba_period_output &&output ) override;

    void callback( nba_specified_output &&output ) override;
};
