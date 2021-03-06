
#include "../../include/criticaleye.hpp"

class [[eosio::contract("usefortest")]] test
    : public criticaleye
{
    struct [[eosio::table]] response
    {
        uint64_t time;
        nba::period_output period_output;
        nba::specified_output specified_output;

        uint64_t primary_key() const {
            return time;
        }
    };

    multi_index<"responses"_n, response> response_index;

public:
    test( name self, name first_receiver, datastream<const char *> ds )
        : criticaleye( self, first_receiver, ds )
        , response_index( self, self.value )
    {}

    DEFAULT_PAY_RECEIVE_ACTION()

    [[eosio::action]]
    void require( string type );

    void callback( nba::period_output &&output ) override;

    void callback( nba::specified_output &&output ) override;
};
