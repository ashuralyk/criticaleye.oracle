
import DayJs from 'dayjs'
import Printer from './printer'

String.prototype.format = function() {
    let args = Array.prototype.slice.call(arguments)
    let i = 0
    return this
    .replace(/%{param}/g, () => {
        return args[i++]
    })
    .replace(/%{time}/g, () => {
        return DayJs.unix(Date.now()).format('YYYY-MM-DD HH:mm:ss')
    })
    .replace(/%{stack}/g, () => {
        var stack = {}
        Error.captureStackTrace(stack, String.prototype.format)
        return stack.stack.split(/\n+/)[2].replace(/(^\s+|\s+$)/, '')
    })
}

// 系统全局配置
const config = {
    oracle: {
        eosio: {
            network: 'http://api-mainnet.starteos.io:80',
            chainId: 'aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906',
        },

        foresight: {
            fifa: {
                path: '/foresight/fifa',
                port: 5001,
                timeout: 5000,
                mspf: 1500,
                contract: {
                    code:   'nbasportsaaa',
                    scope:  'nbasportsaaa',
                    table:  'oracle',
                    upload: 'answer'
                }
            },
            nba: {
                path: '/foresight/nba',
                port: 5002,
                timeout: 5000,
                mspf: 1500,
                contract: {
                    code:   'nbasportsaaa',
                    scope:  'nbasportsaaa',
                    table:  'oracle',
                    upload: 'answer'
                }
            },
            use: ['fifa', 'nba']
        },

        handler: {
            error:      { printer: 'console', type: 'error',   format: '[FATAL(%{time} %{stack})] %{param}' },
            warning:    { printer: 'console', type: 'warning', format: '[WARNING(%{time} %{stack})] %{param}' },
            info:       { printer: 'console', type: 'info',    format: '[INFO(%{time} %{stack})] %{param}' },
            state:      { printer: 'console', type: 'state',   format: '[STATE(%{time})] %{param}' },
            statistics: { printer: 'monitor', type: 'statistics' },
        },
    },

    printer: {
        monitor: {
            path: '/monitor',
            port: 3000
        }
    },

    wallet: {
        local: {
            privateKeys: [
                '5KkAhuTwb4BuguSWPFaPtYBHpMKfBdFBEnGY92uLrdaPJ5dfgNd'
            ]
        },
        remote: {
            path: '/wallet',
            port: 4000,
            timeout: 10000,
            publicKeys: [
                'EOS5VSQsDJY72kWydKuyRyNfHTkQUCZP4NNnHZqa8iC5MGTCbiaSf',
                ''
            ]
        },
        use: 'local'
    }
}

export function getEosio(option) {
    let eosio = config.oracle.eosio[option]
    if (! eosio) {
        throw new Error(`[getEosio] received an unexpected "option" assigned with "${option}"`)
    }
    return eosio
}

export function getForesight(type, option) {
    let foresight = config.oracle.foresight[type]
    if (! foresight) {
        throw new Error(`[getForesight] received an unexpected "type" assigned with "${type}", the expected types are "fifa" and "nba"`)
    }
    if (option) {
        const value = foresight[option]
        if (! value) {
            throw new Error(`[getForesight] received an unexpected "option" assigned with "${option}"`)
        }
        return value
    } else {
        return foresight
    }
}

export function getMonitor(option) {
    let monitor = config.printer.monitor[option]
    if (! monitor) {
        throw new Error(`[getMonitor] received an unexpected "option" assigned with "${option}"`)
    }
    return monitor
}

export function getHandler(type, format = null) {
    let handler = config.oracle.handler[type]
    if (handler == null) {
        throw new Error(`[getHandler] received an unexpected "type" assigned with "${type}"`)
    }
    let printer = Printer.getPrinter(handler.printer)
    return (...logs) => {
        if (format) {
            printer[handler.type](format.format(logs))
        } else if (handler.format) {
            printer[handler.type](handler.format.format(logs))
        } else {
            printer[handler.type](logs)
        }
    }
}

export function getWallet(type, option) {
    let wallet = config.wallet[type]
    if (! wallet) {
        throw new Error(`[getWallet] received an unexpected "type" assigned with "${type}"`)
    }
    if (option) {
        const value = wallet[option]
        if (! value) {
            throw new Error(`[getWallet] received an unexpected "option" assigned with "${option}"`)
        }
        return value
    } else {
        return wallet
    }
}
