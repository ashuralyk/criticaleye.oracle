
import * as Console from './type/console'
import * as Monitor from './type/monitor'

export default function getPrinter(printer) {
    switch (printer) {
        case 'console': return Console
        case 'monitor': Monitor.startServer(); return Monitor
    }
    throw new Error(`[getPrinter] received an unexpected "printer" assigned with "${printer}"`)
}
