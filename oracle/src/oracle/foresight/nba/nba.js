
import Eosio from '../../eosio'
import Wallet from '../../../wallet'
import { getForesight } from '../../../config'
import { makeForesightServer } from '../server'

export function start() {
    console.log('开启NBA监控服务器...')

    // 开启监听服务器
    let foresight = makeForesightServer('nba')

    // 开始循环监视合约状态
    let contract = getForesight('nba', 'contract')
    setInterval(async () => {
        console.log('监控NBA合约...', foresight.state())
        if (foresight.hasFeederOnline()) {
            const ret = await Eosio.getTableRows(contract.code, contract.scope, contract.table)
            if (ret) {

            }
        }
    }, getForesight('nba', 'mspf'))
}
