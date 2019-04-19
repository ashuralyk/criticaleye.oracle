
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

    // [[eosio::action]]
    // void require( name payer, string input_json );
    
    [[eosio::action]]
    void require( name payer, nba::specified::input command );

    [[eosio::action]]
    void response( name payer, checksum256 receipt, string output_json );

    [[eosio::action]]
    void timeout( name payer, checksum256 receipt );

    [[eosio::action]]
    void config( tables::config setting );

private:
    template <int _Opt>
    auto get_config();

    tuple<checksum256, asset> make_receipt( name payer, string &cmd );

    void send_timeout_tx( name payer, checksum256 receipt, bool send = true );

    void send_receipt( name payer, checksum256 receipt, asset bill );
};

/////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
/////////////////////////////////////////////////////////////

template <int _Opt>
inline auto NBA::get_config()
{
    if constexpr ( _Opt == "abandoned_timeout"_m )    return _config.get_or_default({60, 5}).abandoned_timeout;
    if constexpr ( _Opt == "max_oracle_per_payer"_m ) return _config.get_or_default({60, 5}).max_oracle_per_payer;
}

inline tuple<checksum256, asset> NBA::make_receipt( name payer, string &cmd )
{
    string source = payer.to_string() + cmd + to_string(current_time_point().time_since_epoch().count());
    return { sha256(source.c_str(), source.size()), asset(1, symbol("EOS", 4)) };
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
