
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include "../../include/criticaleye.hpp"

using namespace eosio;
using namespace std;

namespace NBA
{

struct Guess
{
    uint8_t type;           // 比赛类型
    uint8_t bet;            // 竞猜方向
    float   score;          // 竞猜值(根据类型type的不同这个值的含义也不同)
    name    creator;        // 竞猜创建者
    name    player;         // 竞猜另一方玩家
    asset   tokenAmount;    // 竞猜下注金额
    name    winner;         // 竞猜胜利方

    bool operator == ( const Guess &o ) {
        return o.type == type && o.bet == bet && o.score == score && o.creator == creator && o.tokenAmount == tokenAmount;
    }
};

struct GuessVector
{
    vector<Guess> guessVector;
};

struct [[eosio::table("data"), eosio::contract("nbasportsaaa")]] Data
{
    uint64_t globalId;          // 唯一自增ID
    string   gameId;            // 比赛唯一ID
    uint8_t  homeTeamScore;     // 主队分数
    uint8_t  awayTeamScore;     // 客队分数
    uint8_t  status;            // 比赛进行状态(0未开始 1进行中 2已结束)

    map<name, GuessVector> guessMap;      // 比赛竞猜列表

    uint64_t primary_key() const {
        return globalId;
    }
};

struct [[eosio::table("config"), eosio::contract("nbasportsaaa")]] Config
{
    float feeRateFail;
    float feeRateWin;
};

}

namespace TABLES
{

typedef multi_index<"data"_n, NBA::Data>   NBAData;
typedef singleton<"config"_n, NBA::Config> NBAConfig;

}

class [[eosio::contract("nbasportsaaa")]] NBASports
    : public criticaleye
{
    TABLES::NBAConfig _config;
    TABLES::NBAData   _nbaData;

public:
    NBASports( name receiver, name code, datastream<const char*> ds )
        : criticaleye( receiver, code, ds )
        , _config( get_self(), get_self().value )
        , _nbaData( get_self(), get_self().value )
    {}

    DEFAULT_PAY_RECEIVE_ACTION()

    [[eosio::action]]
    void config( NBA::Config config );

    [[eosio::action]]
    void erase( string gameId, name creator, uint8_t guessIndex );

    [[eosio::action]]
    void refreash( uint32_t lowerBound, uint32_t upperBound );

    void transfer( name from, name to, asset quantity, string memo );

private:
    void create( string &&param, name creator, asset value );

    void join( string &&param, name player, asset value );

    void callback( nba::period_output &&output ) override;

    void close( NBA::Data &data );

    //////////////////////////////////////////////////
    // 功能性
    //////////////////////////////////////////////////

    float stof( string &view );

    vector<string> split( string &view, char s );

    name getWinner( NBA::Data &data, NBA::Guess &guess );

    template <int tid, typename _Type>
    TABLES::NBAData::const_iterator findDataByType( _Type type ) {
        auto i = _nbaData.end();
        if constexpr ( tid == 0 ) {
            i = find_if( _nbaData.begin(), _nbaData.end(), [&](auto &v) {
                return v.gameId == type;
            });
        } else if constexpr ( tid == 1 ) {
            i = _nbaData.find( type );
        }
        if ( i == _nbaData.end() ) {
            return _nbaData.end();
        } else {
            return i;
        }
    }

    template <int _FeeTy>
    asset getFee( asset quantity )
    {
        auto c = _config.get_or_default( {} );
        if constexpr ( _FeeTy == 0 ) return static_cast<uint32_t>( c.feeRateFail * 100 ) * quantity / 100;
        if constexpr ( _FeeTy == 1 ) return static_cast<uint32_t>( c.feeRateWin * 100 ) * quantity / 100;
        return asset( 0, symbol("EOS", 4) );
    }
};
