
#ifndef _INCLUDE_UTIL_
#define _INCLUDE_UTIL_

#include <array>
#include <string>
#include <eosio/fixed_bytes.hpp>
#include <boost/fusion/include/for_each.hpp>

using namespace std;
using namespace eosio;

namespace util
{

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

}

namespace json
{

template <typename _Type>
struct reflect
{
    static const constexpr bool jsonable = false;
};

template <typename _Read, typename _Write>
struct member
{
    _Read  read;
    _Write write;
};

template <typename _Type>
struct parser
{
    template <typename _Value>
    static string modify( _Value &value ) {
        return to_string( value );
    }

    template <>
    static string modify<string>( string &value ) {
        return "\"" + value + "\"";
    }

    static string to_json( _Type &cmd ) {
        string json = "{";
        boost::fusion::for_each( reflect<_Type>::instance(), [&](auto &v) {

            auto t = v.read( cmd );
            json += get<1>(t) + ":" + modify(get<0>(t)) + ",";
        });
        json[json.size() - 1] = '}';
        if ( json == "}" ) {
            internal_use_do_not_use::eosio_assert( false, "internal error: bad jsonable type." );
        }
        return json;
    }

    static _Type from_json( string_view json ) {

    }
};

#define json_reflect_tuple_impl( type, elem ) \
    json::member<std::function<std::tuple<decltype(type::elem), std::string>(type &)>, std::function<void(type &, decltype(type::elem))>>

#define json_reflect_tuple_impl_comma( r, type, elem ) \
    json_reflect_tuple_impl( type, elem ),

#define json_reflect_tuple( type, elems, lastelem ) \
    static std::tuple< \
        BOOST_PP_SEQ_FOR_EACH( json_reflect_tuple_impl_comma, type, elems ) \
        json_reflect_tuple_impl( type, lastelem ) \
    >

#define json_reflect_lambda_impl( type, elem ) \
    { [](type &i){return std::tuple<decltype(type::elem), std::string>{i.elem, #elem};}, [](type &i, decltype(type::elem) v){i.elem = v;} }

#define json_reflect_lambda_impl_comma( r, type, elem ) \
    json_reflect_lambda_impl( type, elem ),

#define json_reflect_lambda( type, elems, lastelem ) \
    return { \
        BOOST_PP_SEQ_FOR_EACH( json_reflect_lambda_impl_comma, type, elems ) \
        json_reflect_lambda_impl( type, lastelem ) \
    };

#define json_reflect_m( type, elems, lastelem ) \
    template <> struct reflect<type> { \
        json_reflect_tuple( type, elems, lastelem ) \
            instance() { \
                json_reflect_lambda( type, elems, lastelem ) \
            } \
    };

#define json_reflect_s( type, elem ) \
    template <> struct reflect<type> { \
        static std::tuple<json_reflect_tuple_impl(type, elem)> \
            instance() { \
                return { \
                    json_reflect_lambda_impl( type, elem ) \
                }; \
            } \
    };


template <typename _Type>
_Type get_value( string_view json, string_view option )
{

}

}

#endif
