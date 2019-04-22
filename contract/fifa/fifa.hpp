
#ifndef _FIFA_
#define _FIFA_

#include "../criticaleye/criticaleye.hpp"
#include "../include/nba/command.hpp"

class [[eosio::contract(nba_contract)]] NBA
    : public criticaleye<>
{
    struct [[eosio::table]] config
        : public base::config<>
    {};

    struct [[eosio::table]] oracle
        : public base::oracle<>
    {};

    typedef criticaleye<> parent;

public:
    NBA( name self, name first_receiver, datastream<const char *> ds )
        : parent( self, first_receiver, ds )
    {}

    [[eosio::action]]
    void require( name payer, string data_type, vector<char> require_data ) {
        parent::require<nba_period_input, nba_specified_input>( payer, data_type, require_data );
    }

    [[eosio::action]]
    void response( name responser, name payer, checksum256 receipt, vector<char> response_data ) {
        parent::response<nba_period_output, nba_specified_output>( responser, payer, receipt, response_data );
    }

    [[eosio::action]]
    void test( name responser, name payer, checksum256 receipt, nba_period_output response_data/*, nba_specified_output data_2*/ ) {
        response( responser, payer, receipt, pack<util::protocol<nba_period_output>>({
            .generate_time = current_time_point().time_since_epoch().count(),
            .command       = response_data
        }));
    }

    [[eosio::action]]
    void timeout( name payer, checksum256 receipt ) {
        parent::timeout( payer, receipt );
    }

    [[eosio::action]]
    void limit( base::limit limit ) {
        parent::limit( limit );
    }

    [[eosio::action]]
    void auth( name responser, bool add ) {
        parent::auth( responser, add );
    }

    [[eosio::action]]
    void privilege( name payer, bool add ) {
        parent::privilege( payer, add );
    }
};

#endif
