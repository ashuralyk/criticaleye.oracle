
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
            send_timeout_tx( from, (*f).receipt, false );
        }
    });
}

void NBA::require( name payer, uint8_t data_type, vector<char> require_data )
{
    require_auth( payer );
    nba::check_require( data_type, require_data );

    checksum256 receipt;
    asset bill;
    tie(receipt, bill) = make_receipt( payer, string(require_data.data()) );
    if ( auto i = _require_list.find(payer.value); i == _require_list.end() )
    {
        _require_list.emplace( get_self(), [&](auto &v) {
            v.payer = payer;
            v.requirements.push_back({
                .require_type = data_type,
                .require_data = require_data,
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
                .require_type = data_type,
                .require_data = require_data,
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
    send_action( payer, "receipt"_n, make_tuple(get_self(), receipt, bill) );
}

void NBA::response( name payer, checksum256 receipt, vector<char> response_data )
{
    require_auth( payer );

    auto i = _require_list.require_find( payer.value, ("玩家(" + payer.to_string() + ")的请求数据不存在").c_str() );
    if ( auto f = find_if((*i).requirements.begin(), (*i).requirements.end(), [&](auto &r){return r.receipt == receipt;}); f != (*i).requirements.end() )
    {
        nba::check_response( (*f).require_type, response_data );

        // send inline action to tell payer there is a response from oracle
        send_action( payer, "callback"_n, make_tuple(get_self(), receipt, response_data) );
    }
    else
    {
        ROLLBACK( "玩家(" + payer.to_string() + ")下不存在指定的收据信息：" + util::checksum_to_string(receipt) );
    }
}

void NBA::timeout( name payer, checksum256 receipt )
{
    require_auth( get_self() );

    bool clear = false;
    auto i = _require_list.require_find( payer.value, ("合约存在问题：不存在玩家(" + payer.to_string() + ")的请求数据").c_str() );
    _require_list.modify( i, get_self(), [&](auto &v) {
        auto r = find_if( v.requirements.begin(), v.requirements.end(), [&](auto &val){return val.receipt == receipt;} );
        if ( r == v.requirements.end() ) {
            ROLLBACK( "合约存在问题：不存在玩家(" + payer.to_string() + ")的指定收据内容的请求数据" );
        } else {
            v.requirements.erase( r );
        }
        clear = v.requirements.empty();
    });

    if ( clear )
    {
        _require_list.erase( i );
    }
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
