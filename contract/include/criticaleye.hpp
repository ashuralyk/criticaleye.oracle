
#ifndef _INCLUDE_CRITICALEYE_
#define _INCLUDE_CRITICALEYE_

#include <map>
#include <functional>
#include <variant>
#include "./nba/command.hpp"

using namespace std;
using namespace eosio;

class criticaleye
    : public contract
{

using output_0 = nba::period::output;
using output_1 = nba::specified::output;

public:
    criticaleye( name self, name first_receiver, datastream<const char *> ds )
        : contract( self, first_receiver, ds )
    {}

    template <typename _Input>
    checksum256 require( name payer, _Input &&input ) {
        vector<char> packed_data = pack<util::protocol<_Input>>({
            .generate_time = current_time_point().time_since_epoch().count(),
            .command       = input
        });
        action(
            permission_level{ payer, "active"_n },
            util::prototype<_Input>::contract_name,
            "require"_n,
            make_tuple( payer, string(util::prototype<_Input>::type_code), packed_data )
        )
        .send();
        return util::make_receipt<checksum256>( payer, packed_data );
    }

    void pay( name server, checksum256 receipt, asset bill ) {
        require_auth( server );
        if ( bill.amount > 0 ) {
            action(
                permission_level{ get_self(), "active"_n },
                "eosio.token"_n,
                "transfer"_n,
                make_tuple( get_self(), server, bill, util::checksum_to_string(receipt) )
            )
            .send();
        }
    }

    void receive( name server, checksum256 receipt, string type, vector<char> data ) {
        require_auth( server );
        if      ( string_view(type) == util::prototype<output_0>::type_code ) callback( unpack<util::protocol<output_0>>(data).command );
        else if ( string_view(type) == util::prototype<output_1>::type_code ) callback( unpack<util::protocol<output_1>>(data).command );
        else {
            util::rollback( "遇到无法解析的命令类别: \"" + type + "\"" );
        }
    }

    virtual void callback( output_0 &&output ) {
        util::rollback( "你忘记重载 \'nba::period::output\' 回调函数, 请仔细检查，如果你已付款，请联系官方客服." );
    }

    virtual void callback( output_1 &&output ) {
        util::rollback( "你忘记重载 \'nba::specified::output\' 回调函数, 请仔细检查，如果你已付款，请联系官方客服." );
    }
};

#endif
