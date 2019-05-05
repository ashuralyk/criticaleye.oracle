
#ifndef _NBA_
#define _NBA_

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

    struct [[eosio::table]] record
        : public base::record<>
    {};

    typedef criticaleye<> parent;

public:
    NBA( name self, name first_receiver, datastream<const char *> ds )
        : parent( self, first_receiver, ds )
    {}

    [[eosio::action]]
    void require( name payer, string data_type, vector<char> request_data ) {
        parent::require<nba::period_input, nba::specified_input>( payer, data_type, request_data );
    }

    [[eosio::action]]
    void response( name responser, name payer, checksum256 receipt, vector<char> response_data ) {
        parent::response<nba::period_output, nba::specified_output>( responser, payer, receipt, response_data );
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

    [[eosio::action]]
    void ban( name payer, bool add ) {
        parent::ban( payer, add );
    }

    [[eosio::action]]
    void clear() {
        parent::clear();
    }
};

#endif
