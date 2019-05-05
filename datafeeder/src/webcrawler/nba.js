const dayjs = require('dayjs');
const axios = require('axios');
const { transportTencent } =  require('./util');
const { teamId, teamName} = require('./nba_teams');

module.exports = main;

const url = 'https://matchweb.sports.qq.com/matchUnion/list';

function main(arg1, arg2){
    if(typeof arg1 === 'number') {
        return getMatchList(arg1, arg2);
    }
    return getMatchInfo(arg1); 
}

async function getMatchList(startTime, endTime) {
    if(!startTime || !endTime) {
        throw new Error('startTime and endTime needed');
    }
    return getTencentMatchList(startTime, endTime);
}

async function getMatchInfo(mid) {
    const matchInfo = mid.split('-');
    const awayTeamName = teamName(matchInfo[0] >> 0);
    const homeTeamName = teamName(matchInfo[1] >> 0);
    const matchStartTimeUnix = matchInfo[2] >> 0;
    if(!awayTeamName || !homeTeamName || !matchStartTimeUnix) {
        throw new Error('invalid mid :' + mid);
    }
    const matchList = await getTencentMatchList(matchStartTimeUnix, matchStartTimeUnix);
    if(!matchList || !matchList.length) {
        return null;
    }
    const match = matchList.find(m => m.awayTeamName === awayTeamName && m.homeTeamName === homeTeamName) || null;
    return match;
}


async function getTencentMatchList(startTime, endTime) {
    if(typeof startTime === 'number') {
        // unix 时间戳
        if(String(startTime).length < 13) {
            startTime *= 1e3;
            endTime *= 1e3;
        }
    }
    
    startTime = dayjs(startTime).format('YYYY-MM-DD');
    endTime = dayjs(endTime).format('YYYY-MM-DD');

    const rs = await axios.get(url, {
        params: {
            startTime: startTime,
            endTime: endTime,
            columnId: 100000,
            index: 3,
            timestamp: Date.now(),
        }
    })
    return transportTencent(rs.data.data);
}