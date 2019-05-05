
import Socket from 'socket.io-client'
import Config from './src/config'
import Feeder from './src' 

let client = Socket.connect(Config.getOracleServer())

client.on('connect', () => {
    console.log('oracle server connected...')
    client.on('claim', demandData => {
        console.log('claim:', demandData)
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
    })
})
