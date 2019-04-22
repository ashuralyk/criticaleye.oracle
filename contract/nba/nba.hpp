
#ifndef _NBA_
#define _NBA_

#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include "../include/nba/command.hpp"

using namespace eosio;
using namespace std;

template <typename T, T... _Opt>
inline constexpr int operator"" _m ()
{
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "abandoned_timeout" )    return 0;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "max_oracle_per_payer" ) return 1;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "allowed_responsers" )   return 2;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "privileged_payers" )    return 3;
}

namespace tables
{

struct [[eosio::table("config"), eosio::contract(nba_contract)]] config
{
    struct limit
    {
        uint32_t abandoned_timeout;
        uint8_t  max_oracle_per_payer;
    };

    limit limit_config;
    set<name> allowed_responsers;
    set<name> privileged_payers;

    static config make() {
        return {
            .limit_config = {
                .abandoned_timeout    = 60,
                .max_oracle_per_payer = 5
            }
        };
    }
};

struct [[eosio::table("oracle"), eosio::contract(nba_contract)]] oracle
{
    struct requirement
    {
        string       require_type;
        vector<char> require_data;
        int64_t      require_time;
        checksum256  receipt;
        asset        bill;
        bool         payed;
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

typedef multi_index<"oracle"_n, tables::oracle> oracle;
typedef singleton<"config"_n, tables::config>   config;

}

class [[eosio::contract(nba_contract)]] NBA
    : public contract
{
    indices::config _config;
    indices::oracle _require_list;

public:
    NBA( name self, name first_receiver, datastream<const char *> ds )
        : contract( self, first_receiver, ds )
        , _config( self, self.value )
        , _require_list( self, self.value )
    {}

    void transfer( name from, name to, asset quantity, string memo );

    [[eosio::action]]
    void require( name payer, string data_type, vector<char> require_data );

    [[eosio::action]]
    void response( name responser, name payer, checksum256 receipt, vector<char> response_data );

    [[eosio::action]]
    void test( name responser, name payer, checksum256 receipt, nba::period::output response_data );

    [[eosio::action]]
    void timeout( name payer, checksum256 receipt );

    [[eosio::action]]
    void limit( tables::config::limit limit );

    [[eosio::action]]
    void auth( name responser, bool add );

    [[eosio::action]]
    void privilege( name payer, bool add );

private:
    template <int _Opt>
    auto get_config();

    void send_timeout_tx( name payer, checksum256 receipt, bool send = true );

    template <typename ..._Args>
    void send_action( name contract, name method, tuple<_Args...> &&params );

    bool is_privileged( name payer );

    void check_authorization( name responser );
};

/////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
/////////////////////////////////////////////////////////////

template <int _Opt>
inline auto NBA::get_config()
{
    if constexpr ( _Opt == "abandoned_timeout"_m )    return _config.get_or_default(tables::config::make()).limit_config.abandoned_timeout;
    if constexpr ( _Opt == "max_oracle_per_payer"_m ) return _config.get_or_default(tables::config::make()).limit_config.max_oracle_per_payer;
    if constexpr ( _Opt == "allowed_responsers"_m )   return _config.get_or_default(tables::config::make()).allowed_responsers;
    if constexpr ( _Opt == "privileged_payers"_m )    return _config.get_or_default(tables::config::make()).privileged_payers;
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

template <typename ..._Args>
inline void NBA::send_action( name contract, name method, tuple<_Args...> &&params )
{
    action(
        permission_level{ get_self(), "active"_n },
        contract,
        method,
        params
    )
    .send();
}

inline bool NBA::is_privileged( name payer )
{
    return get_config<"allowed_responsers"_m>().count( payer ) > 0;
}

inline void NBA::check_authorization( name responser )
{
    if ( get_config<"allowed_responsers"_m>().count(responser) == 0 )
    {
        util::rollback( "你不在本NBA合约允许的上报人员列表中，请联系官方客服。" );
    }
}
    
#endif
