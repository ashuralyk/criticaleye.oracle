
#ifndef _CRITICALEYE_
#define _CRITICALEYE_

#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <functional>
#include <algorithm>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include "../include/util.hpp"

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

namespace base
{

struct limit
{
    uint32_t abandoned_timeout;
    uint8_t  max_oracle_per_payer;
};

template <
    typename _Limit = limit, 
    typename enable_if<is_base_of<limit, _Limit>::value, int>::type = 0
>
struct config
{
    typedef _Limit limit_t;

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

struct requirement
{
    string       require_type;
    vector<char> require_data;
    int64_t      require_time;
    checksum256  receipt;
    asset        bill;
    bool         payed;
};

template <
    typename _Requirement = requirement, 
    typename enable_if<is_base_of<requirement, _Requirement>::value, int>::type = 0
>
struct oracle
{
    typedef _Requirement requirement_t;

    name payer;
    vector<_Requirement> requirements;

    uint64_t primary_key() const {
        return payer.value;
    }
};

}

template <
    typename _Oracle = base::oracle<>, typename _Config = base::config<>,
    typename enable_if<is_base_of<base::oracle<typename _Oracle::requirement_t>, _Oracle>::value, int>::type = 0,
    typename enable_if<is_base_of<base::config<typename _Config::limit_t>, _Config>::value, int>::type = 0
>
class criticaleye
    : public contract
{
protected:
    struct indices {
        typedef singleton<"config"_n, _Config>   config;
        typedef multi_index<"oracle"_n, _Oracle> oracle;
    };

    typename indices::config _config;
    typename indices::oracle _require_list;

public:
    criticaleye( name self, name first_receiver, datastream<const char *> ds )
        : contract( self, first_receiver, ds )
        , _config( self, self.value )
        , _require_list( self, self.value )
    {}

    void transfer( name from, name to, asset quantity, string memo )
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
                util::rollback( "收据信息(" + memo + ", " + quantity.to_string() + ")不存在" );
            } else {
                (*f).payed = true;
                send_timeout_tx( from, (*f).receipt, false );
                print( "玩家\"" + from.to_string() + "\"" + "已完成支付(收据：" + memo + " <= " + quantity.to_string() + ")" );
            }
        });
    }

    // [[eosio::action]]
    template <typename ..._Inputs>
    void require( name payer, string data_type, vector<char> require_data, function<void(typename _Oracle::requirement_t &)> addon = [](typename _Oracle::requirement_t &){} )
    {
        require_auth( payer );
        int64_t generate_time = util::check_data<_Inputs...>( data_type, require_data );

        checksum256 receipt;
        asset bill;
        tie(receipt, bill) = util::make_receipt<checksum256, asset>( payer, require_data );

        bool privileged = is_privileged( payer );
        if ( auto i = _require_list.find(payer.value); i == _require_list.end() )
        {
            _require_list.emplace( get_self(), [&](auto &v) {
                v.payer = payer;
                v.requirements.push_back({
                    .require_type = data_type,
                    .require_data = require_data,
                    .require_time = generate_time,
                    .receipt      = receipt,
                    .bill         = bill,
                    .payed        = privileged
                });
                addon( v.requirements.back() );
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
                    util::rollback( "创建的请求数已超过上限值" );
                }
                if ( any_of(v.requirements.begin(), v.requirements.end(), [&](auto &r){return r.receipt == receipt;}) ) {
                    util::rollback( "已经发起过同样的请求" );
                }
                // push requirement
                v.requirements.push_back({
                    .require_type = data_type,
                    .require_data = require_data,
                    .require_time = generate_time,
                    .receipt      = receipt,
                    .bill         = bill,
                    .payed        = privileged
                });
                addon( v.requirements.back() );
            });
        }

        if (! privileged)
        {
            // send defered transaction to ensure the payer if payed this requirement
            send_timeout_tx( payer, receipt );

            // send inline action to tell payer the receipt of this requirement
            send_action( payer, "pay"_n, make_tuple(get_self(), receipt, bill) );
        }
        else
        {
            // send inline action to tell payer the receipt of this requirement
            send_action( payer, "pay"_n, make_tuple(get_self(), receipt, "0.0000 EOS") );
        }
    }

    // [[eosio::action]]
    template <typename ..._Outputs>
    void response( name responser, name payer, checksum256 receipt, vector<char> response_data )
    {
        check_authorization( responser );

        auto i = _require_list.require_find( payer.value, ("玩家(" + payer.to_string() + ")的请求数据不存在").c_str() );
        if ( auto f = find_if((*i).requirements.begin(), (*i).requirements.end(), [&](auto &r){return r.receipt == receipt;}); f != (*i).requirements.end() )
        {
            util::check_data<_Outputs...>( (*f).require_type, response_data );

            // send inline action to tell payer there is a response from oracle
            send_action( payer, "receive"_n, make_tuple(get_self(), receipt, (*f).require_type, response_data) );
        }
        else
        {
            util::rollback( "玩家(" + payer.to_string() + ")下不存在指定的收据信息：" + util::checksum_to_string(receipt) );
        }
    }

    // [[eosio::action]]
    void timeout( name payer, checksum256 receipt )
    {
        require_auth( get_self() );

        bool clear = false;
        auto i = _require_list.require_find( payer.value, ("合约存在问题：不存在玩家(" + payer.to_string() + ")的请求数据").c_str() );
        _require_list.modify( i, get_self(), [&](auto &v) {
            auto r = find_if( v.requirements.begin(), v.requirements.end(), [&](auto &val){return val.receipt == receipt;} );
            if ( r == v.requirements.end() ) {
                util::rollback( "合约存在问题：不存在玩家(" + payer.to_string() + ")的指定收据内容的请求数据" );
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

    // [[eosio::action]]
    void limit( base::limit limit )
    {
        require_auth( get_self() );

        auto old = _config.get_or_default( _Config::make() );
        old.limit_config = limit;
        _config.set( old, get_self() );
    }

    // [[eosio::action]]
    void auth( name responser, bool add )
    {
        require_auth( get_self() );

        auto old = _config.get_or_default( _Config::make() );
        if ( add )
        {
            old.allowed_responsers.insert( responser );
        }
        else
        {
            old.allowed_responsers.erase( responser );
        }
    }

    // [[eosio::action]]
    void privilege( name payer, bool add )
    {
        require_auth( get_self() );

        auto old = _config.get_or_default( _Config::make() );
        if ( add )
        {
            old.privileged_payers.insert( payer );
        }
        else
        {
            old.privileged_payers.erase( payer );
        }
    }

protected:
    template <int _Opt>
    auto get_config()
    {
        if constexpr ( _Opt == "abandoned_timeout"_m )    return _config.get_or_default(_Config::make()).limit_config.abandoned_timeout;
        if constexpr ( _Opt == "max_oracle_per_payer"_m ) return _config.get_or_default(_Config::make()).limit_config.max_oracle_per_payer;
        if constexpr ( _Opt == "allowed_responsers"_m )   return _config.get_or_default(_Config::make()).allowed_responsers;
        if constexpr ( _Opt == "privileged_payers"_m )    return _config.get_or_default(_Config::make()).privileged_payers;
    }

    void send_timeout_tx( name payer, checksum256 receipt, bool send = true )
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
    void send_action( name contract, name method, tuple<_Args...> &&params )
    {
        action(
            permission_level{ get_self(), "active"_n },
            contract,
            method,
            params
        )
        .send();
    }

    bool is_privileged( name payer )
    {
        return get_config<"allowed_responsers"_m>().count( payer ) > 0;
    }

    void check_authorization( name responser )
    {
        if ( get_config<"allowed_responsers"_m>().count(responser) == 0 )
        {
            util::rollback( "你不在本NBA合约允许的上报人员列表中，请联系官方客服。" );
        }
    }
};

#endif
