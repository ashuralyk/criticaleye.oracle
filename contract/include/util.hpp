
#ifndef _INCLUDE_UTIL_
#define _INCLUDE_UTIL_

#include <array>
#include <string>
#include <tuple>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <boost/fusion/include/for_each.hpp>

using namespace std;
using namespace eosio;

namespace util
{

template <typename T>
inline void rollback( T &&m )
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

template <typename _Checksum>
string checksum_to_string( _Checksum &checksum )
{
    auto byte_array = checksum.extract_as_byte_array();
    return string( reinterpret_cast<char *>(byte_array.data()), byte_array.size() );
}

template <typename _Checksum>
_Checksum string_to_checksum( string &bytes )
{
    if ( _Checksum::num_words() * 16 == bytes.size() )
    {
        array<typename _Checksum::word_t, _Checksum::num_words()> word_array;
        for ( auto i = 0; i < _Checksum::num_words(); ++i )
        {
            typename _Checksum::word_t word = 0;
            for ( auto j = i * 16; j < (i + 1) * 16; ++j )
            {
                word |= static_cast<uint8_t>(bytes[j]) << (j % 16) * 8;
            }
            word_array[i] = word;
        }
        return _Checksum( word_array );
    }
    else
    {
        return _Checksum();
    }
}

template <typename _Command>
struct protocol
{
    // header
    int64_t generate_time;

    // body
    _Command command;
};

template <typename _Type>
struct prototype {};

#define prototype(command, contract, type) \
    template <> struct util::prototype<command> { \
        static const constexpr name contract_name = name(contract); \
        static const constexpr string_view type_code = type; \
    };

template <typename _First, typename ..._Last>
int64_t check_data( string_view data_type, vector<char> &data )
{
    if ( prototype<_First>::type_code == data_type )
    {
        return unpack<protocol<_First>>(data).generate_time;
    }
    if constexpr ( sizeof...(_Last) > 0 )
    {
        return check_data<_Last...>( data_type, data );
    }
    rollback( "invalid data_type while checking packed command data" );
    return 0;
}

template <typename ..._Options>
auto make_receipt( name payer, vector<char> &data )
{
    if constexpr ( is_same<tuple<_Options...>, tuple<checksum256, asset>>::value )
    {
        string source = payer.to_string() + data.data();
        return make_tuple( sha256(source.c_str(), source.size()), asset(1, symbol("EOS", 4)) );
    }
    else if constexpr ( is_same<tuple<_Options...>, tuple<checksum256>>::value )
    {
        string source = payer.to_string() + data.data();
        return sha256( source.c_str(), source.size() );
    }
    else if constexpr ( is_same<tuple<_Options...>, tuple<asset>>::value )
    {
        return asset( 1, symbol("EOS", 4) );
    }
}

}

// asset operator"" _currency( const char *asset_string )
// {
//     string_view sv( asset_string );
//     auto dot = sv.find( '.' );
//     auto space = sv.find( ' ' );
//     if ( dot == string_view::npos || space == string_view::npos || dot == 0 || space == (sv.size() - 1) || space <= (dot + 1) )
//     {
//         return asset();
//     }
//     else
//     {
//         uint8_t precision = static_cast<uint8_t>( space - dot - 1 );
//         string_view code  = sv.substr( space + 1 );
//         string_view money = sv.substr( 0, space );

//         string amount( money.data(), money.size() );
//         amount.erase( amount.find('.') );       

//         return asset( stoll(amount), symbol(code, precision) );
//     }
// }

#endif
