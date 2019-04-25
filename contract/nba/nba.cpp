
#include "nba.hpp"

extern "C"
{

[[eosio::wasm_entry]]
void apply( uint64_t receiver, uint64_t code, uint64_t action )
{
    if( code == receiver ) 
    {
        switch( action ) 
        {
           EOSIO_DISPATCH_HELPER( NBA, (require)(response)(test)(timeout)(limit)(auth)(privilege)(ban)(clear) )
        }
    }
    else
    {
        if ( code == "eosio.token"_n.value && action == "transfer"_n.value )
        {
            execute_action( name(receiver), name(code), &NBA::transfer );
        }
        else
        {
            util::rollback( "please make sure the callback must be the transfer from eosio.token" );
        }
    }
}

}
