
import Http from 'http'
import SockIO from 'socket.io'
import Config from '../../config'

class DataFeederManager {
    constructor() {
        this.datafeeders = []
        this.receiveData = []
        this.resolove = null
    }

    addDataFeeder(feeder) {
        if (this.datafeeders.indexOf(feeder) > -1) {
            this.datafeeders.push(feeder)
            return true
        } else {
            return false
        }
    }

    delDataFeeder(feeder) {
        let i = this.datafeeders.indexOf(feeder)
        if ( i > -1) {
            this.datafeeders.splice(i, 1)
        }
        return i
    }

    claim(demandData) {
        this.datafeeders.forEach(feeder => feeder.emit('claim', demandData))
    }

    collectData() {
        return this.receiveData.map(pair => pair.data)
    }

    receive(data, feeder) {
        if (this.receiveData.find(pair => pair.feeder === feeder) === undefined) {
            this.receiveData.push({
                data: data,
                feeder: feeder
            })
            if (this.receiveAll()) {
                this.resolove && this.resolove(this.collectData())
            }
        }
    }

    receiveAll() {
        let find = true
        for (feeder of this.datafeeders) {
            if (this.receiveData.find(pair => pair.feeder === feeder) === undefined) {
                find = false
                break
            }
        }
        return find
    }

    setOutsideResolve(resolove) {
        this.resolove = resolove
    }

    onlineFeederNum() {
        return this.datafeeders.length
    }
}

export default {
    makeForesightServer(foresightName) {
        let dataFeederManager = new DataFeederManager()
        let http = Http.createServer()
        let io = SockIO(http)

        io.of(Config.getForesight(foresightName, 'path')).on('connection', datafeeder => {
            dataFeederManager.addDataFeeder(datafeeder)
            console.info(`A datafeeder(${datafeeder.conn.remoteAddress}) client's connection has established...`)

            datafeeder.on('feed', data => {
                dataFeederManager.receive(data, datafeeder)
            })

            datafeeder.on('disconnect', () => {
                dataFeederManager.delDataFeeder(datafeeder)
                console.warn(`A datafeeder(${datafeeder.conn.remoteAddress}) client's connection has closed yet...`)
            })
        })

        http.listen(Config.getForesight(foresightName, 'port'))
        return { 
            io: io,
            hasFeederOnline: () => {
                return dataFeederManager.onlineFeederNum() > 0
            },
            claim: async (demandData) => {
                return new Promise(function(resolve, reject) {
                    dataFeederManager.setOutsideResolve(resolve)
                    dataFeederManager.claim(demandData)
                    setTimeout(reject, Config.getForesight(foresightName, 'timeout'), dataFeederManager.collectData())
                })
            },
            state() {
                return 'online feeders: ' + dataFeederManager.onlineFeederNum()
            }
        }
    }
} 
