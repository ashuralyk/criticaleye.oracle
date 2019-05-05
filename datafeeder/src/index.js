
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
        let dataSet = await Crawl(lowerBound, upperBound)
        if (dataSet) {
            return {
                periodic_games: dataSet.map(v => {
                    return formatChange(v)
                })
            }
        } else {
            return null
        }
    },

    async claimSpecifiedNba(gameId)
    {
        let data = await Crawl(gameId)
        if (data) {
            return {
                specified_game: formatChange(data)
            }
        } else {
            return null
        }
    }

}
