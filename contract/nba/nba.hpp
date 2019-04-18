
#ifndef _NBA_
#define _NBA_

#include <string>
#include <vector>
#include <tuple>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include "../include/nba/command.hpp"
#include "../include/util.hpp"

using namespace eosio;
using namespace std;

template <typename T>
inline void ROLLBACK( T &&m )
{
    if constexpr ( is_same<T, string>::value )
    {
        internal_use_do_not_use::eosio_assert( false, m.c_str() );
    }
    if constexpr ( is_same<T, const char *>::value )
    {
        internal_use_do_not_use::eosio_assert( false, m );
    }
}

template <typename T, T... _Opt>
inline constexpr int operator"" _m ()
{
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "abandoned_timeout" )    return 0;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "max_oracle_per_payer" ) return 1;
}

namespace tables
{

struct [[eosio::table("config"), eosio::contract("oracleos.nba")]] config
{
    uint32_t     abandoned_timeout;
    uint8_t      max_oracle_per_payer;
    vector<name> allowed_responsers;
};

struct [[eosio::table("require"), eosio::contract("oracleos.nba")]] require
{
    struct requirement
    {
        string      require_json;
        int64_t     require_time;
        checksum256 receipt;
        asset       bill;
        bool        payed;
    };

    name payer;
    vector<requirement> requirements;

    uint64_t primary_key() const {
        return payer.value;
    }
};

}

namespace indices
{

typedef multi_index<"require"_n, tables::require> require;
typedef singleton<"config"_n, tables::config>     config;

}

class [[eosio::contract("oracleos.nba")]] NBA
    : public contract
{
    indices::config  _config;
    indices::require _require_list;

public:
    NBA( name self, name first_receiver, datastream<const char *> ds )
        : contract( self, first_receiver, ds )
        , _config( self, self.value )
        , _require_list( self, self.value )
    {}

    void transfer( name from, name to, asset quantity, string memo );

    [[eosio::action]]
    void require( name payer, string input_json );

    [[eosio::action]]
    void response( name payer, checksum256 receipt, string output_json );

    [[eosio::action]]
    void timeout( name payer, checksum256 receipt );

    [[eosio::action]]
    void config( tables::config setting );

private:
    template <int _Opt>
    inline auto get_config() {
        if constexpr ( _Opt == "abandoned_timeout"_m )    return _config.get_or_default({60, 5}).abandoned_timeout;
        if constexpr ( _Opt == "max_oracle_per_payer"_m ) return _config.get_or_default({60, 5}).max_oracle_per_payer;
    }

    template <typename _Cmd>
    tuple<checksum256, asset> make_receipt( name payer, _Cmd &cmd ) {
        string source = payer.to_string() + cmd.to_json() + to_string(current_time_point().time_since_epoch().count());
        return { sha256(source.c_str(), source.size()), asset(1, symbol("EOS", 4)) };
    }

    template <typename _Cmd>
    void push_requirement( name payer, _Cmd command );

    void send_timeout_tx( name payer, checksum256 receipt, bool send = true );

    void send_receipt( name payer, checksum256 receipt, asset bill );
};

/////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
/////////////////////////////////////////////////////////////

template <typename _Cmd>
inline void NBA::push_requirement( name payer, _Cmd command )
{
    checksum256 receipt;
    asset bill;
    tie(receipt, bill) = make_receipt( payer, command );
    if ( auto i = _require_list.find(payer.value); i == _require_list.end() )
    {
        _require_list.emplace( get_self(), [&](auto &v) {
            v.payer = payer;
            v.requirements.push_back({
                .require_json = command.to_json(),
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
                .require_json = command.to_json(),
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

inline void NBA::send_timeout_tx( name payer, checksum256 receipt, bool send /*= true*/ )
{
    uint128_t sender_id = receipt.data()[payer.value % receipt.size()] << 64 | payer.value;
    print( "timeout_tx_sender_id = ", sender_id );
    if ( send )
    {
        transaction tx;
        tx.actions.push_back( action(
            permission_level{ get_self(), "active"_n },
            get_self(),
            "timeout"_n,
            make_tuple( payer, receipt )
        ));
        tx.delay_sec = get_config<"abandoned_timeout"_m>();
        tx.send( sender_id, get_self() );
    }
    else
    {
        cancel_deferred( sender_id );
    }
}

inline void NBA::send_receipt( name payer, checksum256 receipt, asset bill )
{
    action(
        permission_level{ get_self(), "active"_n },
        payer,
        "receipt"_n,
        make_tuple( get_self(), receipt, bill )
    )
    .send();
}
    
#endif
