
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

#define _DEBUG_

template <typename T, T... _Opt>
inline constexpr int operator"" _m ()
{
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "abandoned_timeout" )    return 0;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "max_oracle_per_payer" ) return 1;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "allowed_responsers" )   return 2;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "privileged_payers" )    return 3;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "contract_freezed" )     return 4;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "banned_payers" )        return 5;
    if ( string_view{ detail::to_const_char_arr<_Opt...>::value, sizeof...(_Opt) } == "max_record_per_payer" ) return 6;
}

namespace base
{

struct limit
{
    bool     contract_freezed;
    uint32_t abandoned_timeout;
    uint8_t  max_oracle_per_payer;
    uint8_t  max_record_per_payer;
};

template <
    typename _Limit = limit, 
    typename enable_if<is_base_of<limit, _Limit>::value, int>::type = 0
>
struct config
{
    typedef _Limit limit_t;

    limit limit_config;
    set<name> banned_payers;
    set<name> privileged_payers;
    set<name> allowed_responsers;

    static config make() {
        return {
            .limit_config = {
                .contract_freezed     = false,
                .abandoned_timeout    = 60,
                .max_oracle_per_payer = 5,
                .max_record_per_payer = 5
            }
        };
    }
};

struct request
{
    string       request_type;
    vector<char> request_data;
    int64_t      request_time;
    checksum256  receipt;
    asset        bill;
    bool         payed;
    bool         responsed;
};

template <
    typename _Request = request,
    typename enable_if<is_base_of<request, _Request>::value, int>::type = 0
>
struct oracle
{
    typedef _Request request_t;

    name payer;
    vector<_Request> requests;

    uint64_t primary_key() const {
        return payer.value;
    }
};

struct history
{
    checksum256  receipt;
    string       response_type;
    vector<char> response_data;
    int64_t      response_time;
};

template <
    typename _History = history, 
    typename enable_if<is_base_of<history, _History>::value, int>::type = 0
>
struct record
{
    typedef _History history_t;

    name payer;
    vector<_History> response_history;

    uint64_t primary_key() const {
        return payer.value;
    }
};

}

template <
    typename _Oracle = base::oracle<>, typename _Config = base::config<>, typename _Record = base::record<>,
    typename enable_if<is_base_of<base::oracle<typename _Oracle::request_t>, _Oracle>::value, int>::type = 0,
    typename enable_if<is_base_of<base::config<typename _Config::limit_t>, _Config>::value, int>::type   = 0,
    typename enable_if<is_base_of<base::record<typename _Record::history_t>, _Record>::value, int>::type = 0
>
class criticaleye
    : public contract
{
    typedef typename _Record::history_t history_t;

protected:
    struct indices {
        typedef singleton<"config"_n, _Config>   config;
        typedef multi_index<"oracle"_n, _Oracle> oracle;
        typedef multi_index<"record"_n, _Record> record;
    };

    typename indices::config _config;
    typename indices::oracle _require_list;
    typename indices::record _history_list;

public:
    criticaleye( name self, name first_receiver, datastream<const char *> ds )
        : contract( self, first_receiver, ds )
        , _config( self, self.value )
        , _require_list( self, self.value )
        , _history_list( self, self.value )
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
            auto f = find_if( v.requests.begin(), v.requests.end(), [&](auto &r){return util::checksum_to_string(r.receipt) == memo && r.bill == quantity;} );
            if ( f == v.requests.end() ) {
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
    void require( name payer, string data_type, vector<char> request_data, function<void(_Oracle &)> addon = [](_Oracle &){} )
    {
        check_prohibition( payer );
        int64_t generate_time = util::check_data<_Inputs...>( data_type, request_data );

        checksum256 receipt;
        asset bill;
        tie(receipt, bill) = util::make_receipt<checksum256, asset>( payer, request_data );

        bool privileged = is_privileged( payer );
        if ( auto i = _require_list.find(payer.value); i == _require_list.end() )
        {
            _require_list.emplace( get_self(), [&](auto &v) {
                v.payer = payer;
                v.requests.push_back({
                    .request_type = data_type,
                    .request_data = request_data,
                    .request_time = generate_time,
                    .receipt      = receipt,
                    .bill         = bill,
                    .payed        = privileged,
                    .responsed    = false
                });
                addon( v );
            });
        }
        else
        {
            _require_list.modify( i, get_self(), [&](auto &v) {
                // clear all responsed requests
                auto removed = remove_if( v.requests.begin(), v.requests.end(), [&](auto &r){return r.responsed;} );
                v.requests.erase( removed, v.requests.end() );
                // check validation
                if ( v.requests.size() > get_config<"max_oracle_per_payer"_m>() ) {
                    util::rollback( "创建的请求数已超过上限值" );
                }
                if ( any_of(v.requests.begin(), v.requests.end(), [&](auto &r){return r.receipt == receipt;}) ) {
                    util::rollback( "已经发起过同样的请求" );
                }
                // push request
                v.requests.push_back({
                    .request_type = data_type,
                    .request_data = request_data,
                    .request_time = generate_time,
                    .receipt      = receipt,
                    .bill         = bill,
                    .payed        = privileged,
                    .responsed    = false
                });
                addon( v );
            });
        }

        if ( privileged )
        {
            // send inline action to tell payer the receipt of this request
            send_action( payer, "pay"_n, make_tuple(get_self(), receipt, "0.0000 EOS"_currency) );
        }
        else
        {
            // send defered transaction to ensure the payer if payed this request
            send_timeout_tx( payer, receipt );

            // send inline action to tell payer the receipt of this request
            send_action( payer, "pay"_n, make_tuple(get_self(), receipt, bill) );
        }
    }

    // [[eosio::action]]
    template <typename ..._Outputs>
    void response( name responser, name payer, checksum256 receipt, vector<char> response_data, function<void(history_t &)> addon = [](history_t &){} )
    {
        check_authorization( responser );

        auto i = _require_list.require_find( payer.value, ("玩家(" + payer.to_string() + ")的请求数据不存在").c_str() );
        _require_list.modify( i, get_self(), [&](auto &v) {
            if ( auto f = find_if(v.requests.begin(), v.requests.end(), [&](auto &r){return r.receipt == receipt;});
                    f != v.requests.end() ) {
                // check data and alter state
                int64_t generate_time = util::check_data<_Outputs...>( (*f).request_type, response_data );
                (*f).responsed = true;

                // make response history
                history_t history = {
                    .receipt       = receipt,
                    .response_type = (*f).request_type,
                    .response_data = response_data,
                    .response_time = generate_time
                };
                addon( history );
                make_history( payer, move(history) );

                // send inline action to tell payer there is a response from oracle
                send_action( payer, "receive"_n, make_tuple(get_self(), receipt, (*f).request_type, response_data) );
            } else {
                util::rollback( "玩家(" + payer.to_string() + ")下不存在指定的收据信息：" + util::checksum_to_string(receipt) );
            }
        });
    }

    // [[eosio::action]]
    void timeout( name payer, checksum256 receipt )
    {
        require_auth( get_self() );

        bool clear = false;
        auto i = _require_list.require_find( payer.value, ("合约存在问题：不存在玩家(" + payer.to_string() + ")的请求数据").c_str() );
        _require_list.modify( i, get_self(), [&](auto &v) {
            auto r = find_if( v.requests.begin(), v.requests.end(), [&](auto &val){return val.receipt == receipt;} );
            if ( r == v.requests.end() ) {
                util::rollback( "合约存在问题：不存在玩家(" + payer.to_string() + ")的指定收据内容的请求数据" );
            } else {
                v.requests.erase( r );
            }
            clear = v.requests.empty();
        });

        if ( clear )
        {
            _require_list.erase( i );
        }
    }

    // [[eosio::action]]
    void limit( typename _Config::limit_t &limit )
    {
        alter_config<>( limit );
    }

    // [[eosio::action]]
    void auth( name responser, bool add )
    {
        alter_config<"allowed_responsers"_m>( responser, add );
    }

    // [[eosio::action]]
    void privilege( name payer, bool add )
    {
        alter_config<"privileged_payers"_m>( payer, add );
    }

    // [[eosio::action]]
    void ban( name payer, bool add )
    {
        alter_config<"banned_payers"_m>( payer, add );
    }

    // [[eosio::action]]
    void clear()
    {
        require_auth( permission_level{ get_self(), "owner"_n } );

        _config.remove();
        for (; _require_list.begin() != _require_list.end(); _require_list.erase(_require_list.begin()));
    }

protected:
    template <int _Opt>
    auto get_config()
    {
        if constexpr ( _Opt == "contract_freezed"_m )     return _config.get_or_default(_Config::make()).limit_config.contract_freezed;
        if constexpr ( _Opt == "abandoned_timeout"_m )    return _config.get_or_default(_Config::make()).limit_config.abandoned_timeout;
        if constexpr ( _Opt == "max_oracle_per_payer"_m ) return _config.get_or_default(_Config::make()).limit_config.max_oracle_per_payer;
        if constexpr ( _Opt == "max_record_per_payer"_m ) return _config.get_or_default(_Config::make()).limit_config.max_record_per_payer;
        if constexpr ( _Opt == "banned_payers"_m )        return _config.get_or_default(_Config::make()).banned_payers;
        if constexpr ( _Opt == "privileged_payers"_m )    return _config.get_or_default(_Config::make()).privileged_payers;
        if constexpr ( _Opt == "allowed_responsers"_m )   return _config.get_or_default(_Config::make()).allowed_responsers;
    }

    void send_timeout_tx( name payer, checksum256 receipt, bool send = true )
    {
        uint128_t sender_id = receipt.data()[payer.value % receipt.size()] << 64 | payer.value;
        print( "timeout_tx_sender_id = ", sender_id, ' ' );
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
#ifdef _DEBUG_
        action(
            permission_level{ get_self(), "active"_n },
            contract,
            method,
            params
        )
        .send();
#else
        transaction tx;
        tx.actions.push_back( action (
            permission_level{ get_self(), "active"_n },
            contract,
            method,
            params
        ));
        tx.send( contract.value << 64 | static_cast<uint64_t>(util::now()), get_self() );
#endif
    }

    bool is_privileged( name payer )
    {
        return get_config<"privileged_payers"_m>().count( payer ) > 0;
    }

    void check_authorization( name responser )
    {
        if ( get_config<"contract_freezed"_m>() )
        {
            util::rollback( "本预言机合约已被冻结，请联系官方客服." );
        }
        if ( get_config<"allowed_responsers"_m>().count(responser) == 0 )
        {
            util::rollback( "你不在本预言机合约允许的上报人员列表中，请联系官方客服。" );
        }
        require_auth( responser );
    }

    void check_prohibition( name payer )
    {
        if ( get_config<"contract_freezed"_m>() )
        {
            util::rollback( "本预言机合约已被冻结，请联系官方客服." );
        }
        if ( get_config<"banned_payers"_m>().count(payer) > 0 )
        {
            util::rollback( "你在本预言机合约上的操作已被冻结，请联系官方客服。" );
        }
        require_auth( payer );
    }

    template <int _Which = -1, typename ..._Params>
    void alter_config( _Params&&... params )
    {
        require_auth( get_self() );
        auto value = _config.get_or_default( _Config::make() );

        if constexpr ( is_same<tuple<decay_t<_Params>...>, tuple<name, bool>>::value )
        {
            auto [who, add] = tuple<_Params...>{ params... };
            if (! is_account(who) )
            {
                util::rollback( "账号(" + who.to_string() + ")不存在" );
            }

            name user = who;
            if      constexpr ( _Which == "allowed_responsers"_m ) add ? [&]{value.allowed_responsers.insert(user);}() : [&]{value.allowed_responsers.erase(user);}();
            else if constexpr ( _Which == "privileged_payers"_m )  add ? [&]{value.privileged_payers.insert(user);}()  : [&]{value.privileged_payers.erase(user);}();
            else if constexpr ( _Which == "banned_payers"_m )      add ? [&]{value.banned_payers.insert(user);}()      : [&]{value.banned_payers.erase(user);}();
            else
            {
                print( "please make sure you passed the correct _Which to 'alter_config', check the code" );
            }
        }
        else if constexpr ( is_same<tuple<decay_t<_Params>...>, tuple<typename _Config::limit_t>>::value )
        {
            tie(value.limit_config) = tuple<_Params...>{ params... };
        }
        else
        {
            print( "please make sure you passed the correct _Params to 'alter_config', check the code" );
        }

        _config.set( value, get_self() );
    }

    void make_history( name payer, typename _Record::history_t &&history )
    {
        if ( auto i = _history_list.find(payer.value); i == _history_list.end() )
        {
            _history_list.emplace( get_self(), [&](auto &v) {
                v.payer = payer;
                v.response_history.push_back( history );
            });
        }
        else
        {
            _history_list.modify( i, get_self(), [&](auto &v) {
                v.response_history.push_back( history );
                while ( v.response_history.size() > get_config<"max_record_per_payer"_m>() ) {
                    v.response_history.erase( v.response_history.begin() );
                }
            });
        }
    }
};

#endif
