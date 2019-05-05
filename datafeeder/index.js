
import Socket from 'socket.io-client'
import Config from './src/config'
import Feeder from './src' 

let client = Socket.connect(Config.getOracleServer())

client.on('connect', () => {
    console.log('oracle server connected...')
    client.on('claim', demandData => {
        console.log('claim:', demandData)
<<<<<<< HEAD
        client.emit('feed', {
            generate_time: Date.now(),
            command: {
                periodic_games: [
                    {
                        game_id: 'a-b-cdefg',
                        game_start_time: 100,
                        game_end_time: 0,
                        home_team_id: 6,
                        home_team_score: 56,
                        away_team_id: 8,
                        away_team_score: 61
                    },
                    {
                        game_id: 'ab-c-defgh',
                        game_start_time: 200,
                        game_end_time: 0,
                        home_team_id: 10,
                        home_team_score: 33,
                        away_team_id: 7,
                        away_team_score: 40
                    },
                ]
            }
        })
=======
        (async () => {
            let demand = demandData.data
            let data = null
            switch (demandData.type) {
                case 'nba.period.v1':    data = await Feeder.claimPeriodNba(demand.lower_bound, demand.upper_bound); break
                case 'nba.specified.v1': data = await Feeder.claimSpecifiedNba(demand.game_id); break
            }
            if (data) {
                client.emit('feed', {
                    generate_time: Date.now(),
                    command: data
                })
            } else {
                console.error('error occuered.')
            }
        })()
>>>>>>> 2eedaff774d7bbb9bd84dd793ed18534ccf7e24e
    })
})
