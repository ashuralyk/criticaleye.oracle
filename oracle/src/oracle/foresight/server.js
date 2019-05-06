
import Http from 'http'
import SockIO from 'socket.io'
import Config from '../../config'

String.prototype.hashCode = function() {
    var hash = 0, i = 0, len = this.length;
    while (i < len) {
        hash  = ((hash << 5) - hash + this.charCodeAt(i++)) << 0;
    }
    return (hash + 2147483647) + 1
}

class DataFeederManager {
    constructor() {
        this.datafeeders = []
        this.receiveData = {}
    }

    addDataFeeder(feeder) {
        if (this.datafeeders.indexOf(feeder) < 0) {
            this.datafeeders.push(feeder)
            return true
        } else {
            return false
        }
    }

    delDataFeeder(feeder) {
        let i = this.datafeeders.indexOf(feeder)
        if (i > -1) {
            this.datafeeders.splice(i, 1)
        }
        return i
    }

    claim(demandData, resolve) {
        const hashCode = (JSON.stringify(demandData) + Date.now().toString()).hashCode()
        console.log('hashCode = ', hashCode)
        if (! this.receiveData[hashCode]) {
            this.receiveData[hashCode] = {
                resolve: resolve,
                receive: []
            }
            this.datafeeders.forEach(feeder => feeder.emit('claim', {
                code:   hashCode,
                demand: demandData
            }))
            return hashCode
        } else {
            reslove(null)
        }
    }

    collectData(hashCode) {
        if (this.receiveData[hashCode]) {
            return this.receiveData[hashCode].receive.map(pair => pair.data)
        } else {
            return null
        }
    }

    receive(responseData, feeder) {
        const hashCode = responseData.code
        if (this.receiveData[hashCode]) {
            if (! this.receiveData[hashCode].receive.find(pair => pair.feeder === feeder)) {
                this.receiveData[hashCode].receive.push({
                    data:   responseData.data,
                    feeder: feeder
                })
                if (this.receiveAll(hashCode)) {
                    this.receiveData[hashCode].resolve(this.collectData(hashCode))
                }
            }
        }
    }

    receiveAll(hashCode) {
        if (this.receiveData[hashCode]) {
            let find = true
            for (const feeder of this.datafeeders) {
                if (this.receiveData[hashCode].receive.find(pair => pair.feeder === feeder) === undefined) {
                    find = false
                    break
                }
            }
            return find
        } else {
            return null
        }
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
            const ret = dataFeederManager.addDataFeeder(datafeeder)
            console.info(`A datafeeder(${datafeeder.conn.remoteAddress}) client's connection has established... ${ret}`)

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
                    const hashCode = dataFeederManager.claim(demandData, resolve)
                    setTimeout(reject, Config.getForesight(foresightName, 'timeout'), dataFeederManager.collectData(hashCode))
                })
            },
            state() {
                return 'online feeders: ' + dataFeederManager.onlineFeederNum()
            }
        }
    }
} 
