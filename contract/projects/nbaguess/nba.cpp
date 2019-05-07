
#include <functional>
#include "nba.hpp"

void NBASports::config( NBA::Config config )
{
    require_auth( get_self() );

    check( config.feeRateFail > 0.f, "the 'feeRateFail' must be greater than 0" );
    check( config.feeRateWin  > 0.f, "the 'feeRateWin' must be greater than 0" );
    _config.set( config, get_self() );
}

void NBASports::erase( string gameId, name creator, uint8_t guessIndex )
{
    require_auth( get_self() );

    auto i = find_if( _nbaData.begin(), _nbaData.end(), [&](auto &v) {
        return v.gameId == gameId;
    });

    if ( i == _nbaData.end() )
    {
        util::rollback( "there isn't '" + gameId + "' in the data set" );
    }

    auto &guessMap = (*i).guessMap;
    if ( auto j = guessMap.find(creator); j == guessMap.end() )
    {
        util::rollback( "there isn't creator " + creator.to_string() + " in '" + gameId + "'" );
    }
    else
    {
        auto &guessVector = (*j).second.guessVector;
        if ( guessVector.size() < guessIndex )
        {
            util::rollback( "the index is beyond the creator's guesses" );
        }

        auto &nba = guessVector[guessIndex];
        print( nba.creator, ", ", nba.player, ", ", nba.tokenAmount );

        if ( nba.winner == name() )
        {
            // 返回创建者游戏币
            action(
                permission_level{ get_self(), "active"_n },
                "eosio.token"_n,
                "transfer"_n,
                make_tuple( get_self(), nba.creator, nba.tokenAmount, "您在AAASports上创建的竞猜{" + gameId + "}已被删除，退还抵押的EOS" )
            ).send();

            // 返回加入者游戏币
            if ( nba.player != name() )
            {
                action(
                    permission_level{ get_self(), "active"_n },
                    "eosio.token"_n,
                    "transfer"_n,
                    make_tuple( get_self(), nba.player, nba.tokenAmount, "您在AAASports上加入的竞猜{" + gameId + "}已被删除，退还抵押的EOS" )
                ).send();
            }
        }

        // 删除竞猜记录
        _nbaData.modify( i, get_self(), [&](auto &v) {
            auto &g = v.guessMap[creator];
            g.guessVector.erase( g.guessVector.begin() + guessIndex );
            if ( g.guessVector.empty() )
            {
                v.guessMap.erase( creator );
            }
        });
    }
}

void NBASports::refreash( uint32_t lowerBound, uint32_t upperBound )
{
    require_auth( get_self() );

    criticaleye::require<nba::period_input>({
        .lower_bound = lowerBound,
        .upper_bound = upperBound
    });
}

void NBASports::transfer( name from, name to, asset quantity, string memo )
{
    if ( from == get_self() || to != get_self() || quantity.symbol != symbol("EOS", 4) )
    {
        return;
    }

    if ( from == to )
    {
        util::rollback( "invalid transfer params" );
    }

    if ( memo.substr(0, strlen("create")) == "create" )
    {
        create( memo.substr(sizeof("create")), from, quantity );
    }
    else if ( memo.substr(0, strlen("join")) == "join" )
    {
        join( memo.substr(sizeof("join")), from, quantity );
    }
    else
    {
        util::rollback( "invalid memo: must go with 'create' or 'join' as the fist word" );
    }
}

void NBASports::create( string &&param, name creator, asset value )
{
    vector<string> params = split( param, '|' );
    if ( params.size() != 4 || stoul(params[1]) > 1 || stoul(params[2]) > 2 )
    {
        util::rollback( "invalid 'create' memo: create|mid|bet[=0,1]|type[=0,1,2]|score|" );
    }

    print( params[0], ", ", params[1], ", ", params[2], ", ", params[3] );

    // 检查数据集合
    auto i = findDataByType<0>( params[0] );

    if ( i == _nbaData.end() )
    {
        util::rollback( "the 'mid' from the memo doesn't exist in the data set" );
    }

    if ( (*i).status != 0 )
    {
        util::rollback( "the game represented by this 'mid' from the memo has closed yet" );
    }

    // 修改数据
    _nbaData.modify( i, get_self(), [&](auto &v) {
        NBA::Guess guess = {
            .bet         = static_cast<uint8_t>( stoul(params[1]) ),
            .type        = static_cast<uint8_t>( stoul(params[2]) ),
            .score       = stof(params[3]),
            .creator     = creator,
            .tokenAmount = value
        };
        if ( auto j = v.guessMap.find(creator); j != v.guessMap.end() ) {
            if ( any_of((*j).second.guessVector.begin(), (*j).second.guessVector.end(), [&](auto &g){return g == guess;}) ) {
                util::rollback( "the 'mid' with this guess type is already created by " + creator.to_string() );
            }
        }
        v.guessMap[creator].guessVector.push_back( guess );
    });
}

void NBASports::join( string &&param, name player, asset value )
{
    vector<string> params = split( param, '|' );
    if ( params.size() != 3 )
    {
        util::rollback( "invalid 'join' memo: join|mid|creator|which|" );
    }
   
    // 检查数据集合
    auto i = findDataByType<0>( params[0] );

    if ( i == _nbaData.end() )
    {
        util::rollback( "the 'mid' from the memo doesn't exist in the data set" );
    }

    if ( (*i).status != 0 )
    {
        util::rollback( "the game represented by this 'mid' from the memo has closed yet" );
    }

    // 修改数据
    _nbaData.modify( i, get_self(), [&](auto &v) {
        if ( auto j = v.guessMap.find(name(params[1])); j == v.guessMap.end() ) {
            util::rollback( "there is no creator " + params[1] + " under this 'mid'" );
        } else {
            size_t which = static_cast<size_t>( stoul(params[2]) );
            if ( (*j).second.guessVector.size() < which ) {
                util::rollback( "the 'which' is beyond the creator's guesses" );
            }
            NBA::Guess &g = (*j).second.guessVector[which];
            if ( g.player != name() ) {
                util::rollback( "this guess is already on playing, and the player is " + g.player.to_string() );
            }
            if ( g.tokenAmount != value ) {
                util::rollback( "the imported token amount doesn't match this guess's" );
            }
            if ( g.creator == player ) {
                util::rollback( "you can't play with yourself" );
            }
            if ( g.winner != name() ) {
                util::rollback( "this guess already has a winner(" + g.winner.to_string() + "), so refuse to join" );
            }
            g.player = player;
        }
    });
}

void NBASports::callback( nba::period_output &&output )
{
    auto &gameSet = output.periodic_games;

    for ( auto &game : gameSet )
    {
        print( " > ", game.game_id );
    }

    // 修改已有比赛状态并删除被遗忘的比赛
    for ( auto i = _nbaData.begin(); i != _nbaData.end(); )
    {
        if ( auto j = find_if(gameSet.begin(), gameSet.end(), [&](auto &v){return v.game_id == (*i).gameId;}); j != gameSet.end() ) {
            _nbaData.modify( i, get_self(), [&](auto &v) {
                v.homeTeamScore = (*j).home_team_score;
                v.awayTeamScore = (*j).away_team_score;
                // 比赛刚好结束
                if ( v.status < 2 && (*j).status == 2 ) {
                    print( " close(", v.gameId ,")" );
                    close( v );
                }
                v.status = (*j).status;
            });
        } else {
            if ( (*i).status == 2 && (*i).homeTeamScore > 0 && (*i).awayTeamScore > 0 ) {
                print( " erase(", (*i).gameId ,")" );
                i = _nbaData.erase( i ); 
                continue;
            }
        }
        ++i;
    }

    // 记录起始全局ID
    uint64_t nextGlobalId = 0;
    if ( _nbaData.begin() != _nbaData.end() )
    {
        nextGlobalId = (*_nbaData.rbegin()).globalId + 1;
    }

    // 添加额外的比赛
    for_each( gameSet.begin(), gameSet.end(), [&](auto &game) {
        if ( all_of(_nbaData.begin(), _nbaData.end(), [&](auto &v){return v.gameId != game.game_id;}) ) {
            _nbaData.emplace( get_self(), [&](auto &v) {
                v.globalId      = nextGlobalId++;
                v.gameId        = game.game_id;
                v.homeTeamScore = game.home_team_score;
                v.awayTeamScore = game.away_team_score;
                v.status        = game.status;
            });
        }
    });
}

void NBASports::close( NBA::Data &data )
{
    for ( auto &guess : data.guessMap )
    {
        for ( auto &v : guess.second.guessVector )
        {
            // 竞猜流局
            if ( v.player == name() )
            {
                v.winner = v.creator;
                asset payback = v.tokenAmount - getFee<0>( v.tokenAmount );
                action(
                    permission_level{ get_self(), "active"_n },
                    "eosio.token"_n,
                    "transfer"_n,
                    make_tuple( get_self(), v.creator, payback, "您在AAASports上的竞猜{" + data.gameId + "}已结束，返还无人参加情况下的EOS(已扣除手续费)" )
                ).send();
            }
            // 正常结束竞猜
            else
            {
                v.winner = getWinner( data, v );
                asset payback = 2 * (v.tokenAmount - getFee<1>( v.tokenAmount ));
                action(
                    permission_level{ get_self(), "active"_n },
                    "eosio.token"_n,
                    "transfer"_n,
                    make_tuple( get_self(), v.winner, payback, "恭喜，您在AAASports上赢得竞猜{" + data.gameId + "}，发放EOS(已扣除手续费)" )
                ).send();
            }
        }
    }
}

float NBASports::stof( string &view )
{
    const char *s = view.c_str();
    float rez = 0, fact = 1;
    if ( *s == '-' )
    {
        s++;
        fact = -1;
    }

    for ( int point_seen = 0; *s; s++ )
    {
        if ( *s == '.' )
        {
            point_seen = 1; 
            continue;
        }

        int d = *s - '0';
        if ( d >= 0 && d <= 9 )
        {
            if ( point_seen ) fact /= 10.0f;
            rez = rez * 10.0f + (float)d;
        }
    }

    return rez * fact;
}

vector<string> NBASports::split( string &view, char s )
{
    vector<string> params;
    for ( uint32_t i = view.find(s), j = 0; i != string::npos; j = i + 1, i = view.find(s, j) )
    {
        params.push_back( view.substr(j, i - j) );
    }
    return params;
}

name NBASports::getWinner( NBA::Data &data, NBA::Guess &guess )
{
    function<bool()> creatorWin;
    switch ( guess.type )
    {
        // 独赢
        case 0:
        {
            creatorWin = [&]() {
                if ( guess.bet == 0 ) {
                    return data.homeTeamScore > data.awayTeamScore;
                } else {
                    return data.homeTeamScore < data.awayTeamScore;
                }
            };
        }
        break;
        // 总分
        case 1:
        {
            creatorWin = [&]() {
                if ( guess.bet == 0 ) {
                    return static_cast<float>(data.homeTeamScore + data.awayTeamScore) > guess.score;
                } else {
                    return static_cast<float>(data.homeTeamScore + data.awayTeamScore) < guess.score;
                }
            };
        }
        break;
        // 分差
        case 2:
        {
            creatorWin = [&]() {
                if ( guess.bet == 0 ) {
                    return static_cast<float>(data.homeTeamScore) - static_cast<float>(data.awayTeamScore) > guess.score;
                } else {
                    return static_cast<float>(data.homeTeamScore) - static_cast<float>(data.awayTeamScore) < guess.score;
                }
            };
        }
        break;
    }

    return creatorWin() ? guess.creator : guess.player;
}

extern "C"
{

[[eosio::wasm_entry]]
void apply( uint64_t receiver, uint64_t code, uint64_t action )
{
    if( code == receiver )
    {
        switch( action )
        {
            EOSIO_DISPATCH_HELPER( NBASports, (pay)(receive)(config)(erase)(refreash) )
        }
    }
    else
    {
        if ( code == "eosio.token"_n.value && action == "transfer"_n.value )
        {
            execute_action( name(receiver), name(code), &NBASports::transfer );
        }
    }
}

}
