/**
 * match 对象标准格式：
 * mid
 * awayTeamId
 * homeTeamId
 * awayTeamName
 * homeTeamName
 * startTime
 * awayTeamScore
 * homeTeamScore
 * startTimeUnix
 * status  比赛状态：0未开始、1比赛中、2已结束
 */
const dayjs = require('dayjs');
const { teamId, teamName} = require('./nba_teams');

module.exports = {
    getMid,
    transportTencent
}

function getMid(awayTeamId, homeTeamId, startTimeUnix) {
    if (typeof awayTeamId === 'object') {
        const game = awayTeamId;
        return [game.awayTeamId, game.homeTeamId, game.startTimeUnix].join('-');
    }
    return [awayTeamId, homeTeamId, startTimeUnix].join('-');
}

function transportTencent(data) {
    return Array.prototype.concat.apply([], Object.values(data || {}))
    .map(m => {
        const homeTeamId = teamId(m.rightName);
        const awayTeamId = teamId(m.leftName);
        const startTimeUnix = dayjs(m.startTime).unix();
        return {
            mid: getMid(awayTeamId, homeTeamId, startTimeUnix),
            awayTeamId,
            homeTeamId,
            awayTeamName: m.leftName,
            homeTeamName: m.rightName,
            startTime: m.startTime,
            startTimeUnix,
            awayTeamScore: m.leftGoal >> 0,
            homeTeamScore: m.rightGoal >> 0,
            status: m.matchPeriod >> 0
        }
    }).filter(m => {
        if(m.awayTeamId == undefined || m.homeTeamId == undefined) {
            console.warn('invalid match:', m);
            return false;
        }
        return true;
    });
}