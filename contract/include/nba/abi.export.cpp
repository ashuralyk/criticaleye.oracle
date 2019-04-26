
#include <eosio/eosio.hpp>
#include "command.hpp"

class [[eosio::contract(nba_contract)]] abi_export
    : public contract
{
public:
    protocol( nba_period_input, nba::period_input )
    protocol( nba_period_output, nba::period_output )
    protocol( nba_specified_input, nba::specified_input )
    protocol( nba_specified_output, nba::specified_output )

    abi_export( name self, name first_receiver, datastream<const char *> ds )
        : contract( self, first_receiver, ds )
    {}

    [[eosio::action]]
    void commands( nba_period_input, nba_period_output, nba_specified_input, nba_specified_output ){}
};

EOSIO_DISPATCH( abi_export, (commands) )
