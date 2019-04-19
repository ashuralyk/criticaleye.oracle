
#include <algorithm>
#include "nba.hpp"

void NBA::transfer( name from, name to, asset quantity, string memo )
{
    if ( from == get_self() || to != get_self() || from == to )
    {
        return;
    }

    if ( quantity.symbol != symbol("EOS", 4) )
    {
        return;
    }

    // find payer's receipt and deal with it
    auto i = _require_list.require_find( from.value, ("玩家(" + from.to_string() + ")的请求数据不存在").c_str() );

    _require_list.modify( i, get_self(), [&](auto &v) {
        auto f = find_if( v.requirements.begin(), v.requirements.end(), [&](auto &r){return util::checksum_to_string(r.receipt) == memo && r.bill == quantity;} );
        if ( f == v.requirements.end() ) {
            ROLLBACK( "收据信息(" + memo + ", " + quantity.to_string() + ")不存在" );
        } else {
            (*f).payed = true;
        }
    });
}

void NBA::require( name payer, nba::specified::input command )
{
    require_auth( payer );

    string input_json = json::parser<nba::specified::input>::to_json( command );

    checksum256 receipt;
    asset bill;
    tie(receipt, bill) = make_receipt( payer, input_json );
    if ( auto i = _require_list.find(payer.value); i == _require_list.end() )
    {
        _require_list.emplace( get_self(), [&](auto &v) {
            v.payer = payer;
            v.requirements.push_back({
                .require_json = input_json,
                .require_time = current_time_point().time_since_epoch().count(),
                .receipt      = receipt,
                .bill         = bill,
                .payed        = false
            });
        });
    }
    else
    {
        _require_list.modify( i, get_self(), [&](auto &v) {
            // clear all payed requirements
            auto removed = remove_if( v.requirements.begin(), v.requirements.end(), [&](auto &r){return r.payed;} );
            v.requirements.erase( removed, v.requirements.end() );
            // check validation
            if ( v.requirements.size() > get_config<"max_oracle_per_payer"_m>() ) {
                ROLLBACK( "创建的请求数已超过上限值" );
            }
            if ( any_of(v.requirements.begin(), v.requirements.end(), [&](auto &r){return r.receipt == receipt;}) ) {
                ROLLBACK( "已经发起过同样的请求" );
            }
            // push requirement
            v.requirements.push_back({
                .require_json = input_json,
                .require_time = current_time_point().time_since_epoch().count(),
                .receipt      = receipt,
                .bill         = bill,
                .payed        = false
            });
        });
    }

    // send defered transaction to ensure the payer if payed this require
    send_timeout_tx( payer, receipt );

    // send inline action to tell payer the receipt of this requirement
    send_receipt( payer, receipt, bill );

    print( "string(receipt) = ", util::checksum_to_string(receipt) );
}

void NBA::response( name payer, checksum256 receipt, string output_json )
{
    require_auth( payer );
}

void NBA::timeout( name payer, checksum256 receipt )
{
    require_auth( get_self() );
    auto i = _require_list.require_find( payer.value, ("合约存在问题：不存在玩家(" + payer.to_string() + ")的请求数据").c_str() );
    _require_list.modify( i, get_self(), [&](auto &v) {
        auto r = find_if( v.requirements.begin(), v.requirements.end(), [&](auto &val){return val.receipt == receipt;} );
        if ( r == v.requirements.end() ) {
            ROLLBACK( "合约存在问题：不存在玩家(" + payer.to_string() + ")的指定收据内容的请求数据" );
        } else {
            v.requirements.erase( r );
        }
    });
}

void NBA::config( tables::config setting )
{
    require_auth( get_self() );
    _config.set( setting, get_self() );
}

///////////////////////////////////////////////////////
// ENTRY
///////////////////////////////////////////////////////

extern "C"
{

[[eosio::wasm_entry]]
void apply( uint64_t receiver, uint64_t code, uint64_t action )
{
    if( code == receiver ) 
    {
        switch( action ) 
        {
           EOSIO_DISPATCH_HELPER( NBA, (require)(response)(timeout)(config) )
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
            ROLLBACK( "please make sure the callback must be the transfer from eosio.token" );
        }
    }
}

}
