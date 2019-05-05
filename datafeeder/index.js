
import Socket from 'socket.io-client'
import Config from './src/config'

let client = Socket.connect(Config.getOracleServer())

client.on('connect', () => {
    console.log('oracle server connected...')
    client.on('claim', demandData => {
        console.log('claim:', demandData)
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
    })
})
