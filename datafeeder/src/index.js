
import Crawl from './webcrawler/nba'

function formatChange(web)
{
    return {
        game_id:         web.mid,
        game_start_time: web.startTimeUnix,
        home_team_id:    web.homeTeamId,
        home_team_score: web.homeTeamScore,
        away_team_id:    web.awayTeamId,
        away_team_score: web.awayTeamScore,
        status:          web.status
    }
}

export default {

    async claimPeriodNba(lowerBound, upperBound)
    {
        console.log('lowerBound =', lowerBound, 'upperBound =', upperBound)
        let dataSet = await Crawl(lowerBound, upperBound)
        if (dataSet) {
            return {
                periodic_games: dataSet.filter(v => v.awayTeamId).map(v => {
                    console.log('v =', v)
                    return formatChange(v)
                })
            }
        } else {
            return null
        }
    },

    async claimSpecifiedNba(gameId)
    {
        console.log('gameId =', gameId)
        let data = await Crawl(gameId)
        console.log('data =', data)
        if (data) {
            return {
                specified_game: formatChange(data)
            }
        } else {
            return null
        }
    }

}
