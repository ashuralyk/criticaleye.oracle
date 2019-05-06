
import Socket from 'socket.io-client'
import Config from './src/config'
import Feeder from './src' 

let client = Socket.connect(Config.getOracleServer())

async function webcrawl(client, code, demand) {
    let command = demand.data.command
    let feed = null
    switch (demand.type) {
        case 'nba.period.v1':    feed = await Feeder.claimPeriodNba(command.lower_bound, command.upper_bound); break
        case 'nba.specified.v1': feed = await Feeder.claimSpecifiedNba(command.game_id); break
        default: {
            console.error('unexpected type code', demand.type)
            return
        }
    }
    if (feed) {
        client.emit('feed', {
            code: code,
            data: {
                generate_time: Date.now(),
                command: feed
            }
        })
    } else {
        console.error('error occuered.')
    }
}

client.on('connect', () => {
    console.log('oracle server connected...')
    client.on('claim', demandData => {
        console.log('claim:', demandData)
        webcrawl(client, demandData.code, demandData.demand)
    })
})
