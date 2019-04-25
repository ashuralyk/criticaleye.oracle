
import Eosio from '../../eosio'
import Wallet from '../../../wallet'
import { getForesight } from '../../../config'
import { makeForesightServer } from '../server'

class PayerManager
{
    constructor( foresight ) {
        this.payment = {}
        this.foresight = foresight
    }

    require( payer, receipt, requestType, packedRequestData ) {
        // 解码二进制数据
        const requestData = fromRequest( requestType, packedRequestData );
        if (!requestData) {
            return
        }
        // 加入管理器
        if (this.payment[payer]) {
            if (!this.payment[payer][receipt]) {
                this.payment[payer][receipt] = {
                    type: requestType,
                    data: packedRequestData
                }
            } else {
                // 跳过已经处理过的收据信息
                return;
            }
        } else {
            this.payment[payer] = {
                receipt: {
                    type: requestType,
                    data: packedRequestData
                }
            }
        }
        // 发送爬虫请求
    }

    response( payer, receipt, responseType, packedResponseData ) {
        
    }

    fromRequest( type, packedData ) {

    }

    toResponse( type, jsonData ) {

    }

    async sendRequest( type, jsonData ) {
        
    }
}

export function start() {
    console.log('开启NBA监控服务器...')

    // 开启监听服务器
    let foresight = makeForesightServer('nba')
    let payerManager = new PayerManager( foresight )

    // 开始循环监视合约状态
    let contract = getForesight('nba', 'contract')
    setInterval(async () => {
        console.log('监控NBA合约...', foresight.state())
        if (foresight.hasFeederOnline()) {
            const ret = await Eosio.getTableRows(contract.code, contract.scope, contract.table)
            if (ret) {
                console.info( 'ret = ', ret );
                for (row of ret.rows) {
                    for (request of row.requests) {
                        if (request.payed && !request.responsed) {
                            payerManager.require(row.payer, request.receipt, request.request_type, request.request_data)
                        }
                    }
                }
            }
        }
    }, getForesight('nba', 'mspf'))
}
