
import Http from 'http'
import SockIO from 'socket.io'
import { getMonitor } from '../../config'

// 监控器管理器
class MonitorManager {
    constructor() {
        this.monitors = []
    }

    addMonitor(monitor) {
        if (this.monitors.indexOf(monitor) > -1) {
            this.monitors.push(monitor)
        }
    }

    delMonitor(monitor) {
        let i = this.monitors.indexOf(monitor)
        if ( i !== -1 ) {
            this.monitors.splice(i, 1)
        }
        return i
    }

    send(log, type) {
        if (this.monitors.length > 0) {
            this.monitors.forEach(client => {
                client.emit(type, log)
            })
        } else {
            console.warn(`NO monitor client has been connected <= ${log}`)
        }
    }
}

let monitorManager = new MonitorManager()
let http = null
let io = null

export function startServer() {
    if (http || io) {
        return
    }

    http = Http.createServer()
    io = SockIO(http)

    // 配置服务器连接信息
    io.of(getMonitor('path')).on('connection', monitor => {
        console.info(`A monitor(${monitor.conn.remoteAddress}) client's connection has established...`)

        monitor.on('disconnect', () => {
            monitorManager.delMonitor(monitor)
            console.warn(`A monitor(${monitor.conn.remoteAddress}) client's connection has closed yet...`)
        })

        monitorManager.addMonitor(monitor)
        monitor.emit('hello', 'permitted')
    })

    http.listen(getMonitor('port'))
    console.log('Remote monitor server is running...')
}

// 导出接口
export function error(log) {
    monitorManager.send(log, 'error')
    return false
}

export function warning(log) {
    monitorManager.send(log, 'warning')
    return true
}

export function info(log) {
    monitorManager.send(log, 'info')
    return true
}

export function state(log) {
    monitorManager.send(log, 'state')
    return true
}

export function statistics(log) {
    monitorManager.send(log, 'statistics')
    return true
}
