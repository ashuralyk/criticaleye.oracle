
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

template <uint8_t _Type>
struct command
{
    static const constexpr uint8_t type = _Type;
};

}

#endif
