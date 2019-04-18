
import Socket from 'socket.io-client'
import Config from './src/config'

let client = Socket.connect(Config.getOracleServer())

client.on('connect', () => {
    console.log('oracle server connected...')
    client.on('claim', demandData => {
        console.log('claim:', demandData)
    })
})
