
import Eosio from '../../eosio'
import Wallet from '../../../wallet'
import Config from '../../../config'
import Server from '../server'

class PayerManager
{
    constructor( foresight ) {
        this.payment = {}
        this.foresight = foresight
        this.abiTypes = Eosio.generateAbiTypes(require('./nba.abi.json'))
        this.contract = Config.getForesight('nba', 'contract')
    }

    require( payer, receipt, requestType, packedRequestHexData ) {
        // 解码二进制数据
        const requestData = this._fromPackedRequest( requestType, packedRequestHexData )
        if (!requestData) {
            console.warn(`require 函数执行异常, 用户收据：${payer}(${receipt})`)
            return
        }
        // console.info('requestData =', requestData)
        // 加入管理器
        if (this.payment[payer]) {
            if (!this.payment[payer][receipt]) {
                this.payment[payer][receipt] = {
                    type: requestType,
                    data: packedRequestHexData
                }
            } else {
                // 跳过已经处理过的收据信息
                // console.log('skipping...')
                return;
            }
        } else {
            this.payment[payer] = {}
            this.payment[payer][receipt] = {
                type: requestType,
                data: packedRequestHexData
            }
        }
        // 发送爬虫请求
        this._claim( payer, receipt, requestType, requestData ).catch(Config.getHandler('trycatch'))
    }

    async _response( payer, receipt, packedResponseHexData ) {
        const transaction = {
            actions: [{
                account: this.contract.code,
                name: this.contract.action,
                authorization: [{
                    actor: this.contract.actor,
                    permission: 'active'
                }],
                data: {
                    responser: this.contract.actor,
                    payer: payer,
                    receipt: receipt,
                    response_data: packedResponseHexData
                }
            }]
        }
        const signedTransaction = await Wallet.sign(transaction)
        if (signedTransaction) {
            if (await Eosio.pushTransaction(signedTransaction)) {
                if (this.payment[payer]) {
                    delete this.payment[payer][receipt]
                }
                return
            }
        }
        console.warn(`_response 函数执行异常, 用户收据：${payer}(${receipt})`)
    }

    _fromPackedRequest( type, packedHexData ) {
        const abiType = this.contract.inputs[type]  
        if (abiType) {
            let packedData = []
            for (let i = 0; i < packedHexData.length; i += 2) {
                packedData.push(parseInt(packedHexData[i] + packedHexData[i + 1], 16))
            }
            return Eosio.deserializeByAbiType(this.abiTypes, abiType, packedData)
        } else {
            return null
        }
    }

    _toPackedResponse( type, unpackedData ) {
        const abiType = this.contract.outputs[type]  
        if (abiType) {
            return Eosio.serializeByAbiType(this.abiTypes, abiType, unpackedData)
        } else {
            return null
        }
    }

    async _claim( payer, receipt, requestType, requestData ) {
        const responseJsonDataCollection = await this.foresight.claim({
            type: requestType,
            data: requestData
        })
        console.log('responseJsonDataCollection =', responseJsonDataCollection)
        if (responseJsonDataCollection && responseJsonDataCollection.length > 0) {
            const packedResponseHexData = this._toPackedResponse(requestType, responseJsonDataCollection[0])
            // console.log('pack(JsonData) =', packedResponseData)
            if (packedResponseHexData) {
                this._response( payer, receipt, packedResponseHexData )
                return
            }
        }
        console.warn(`_claim 函数执行异常, 删除用户收据：${payer}(${receipt})`)
        if (this.payment[payer]) {
            delete this.payment[payer][receipt]
        }
    }
}

export default {
    start() {
        console.log('开启NBA监控服务器...')

        // 开启监听服务器
        let foresight = Server.makeForesightServer('nba')
        let payerManager = new PayerManager( foresight )

        // 开始循环监视合约状态
        let contract = Config.getForesight('nba', 'contract')
        setInterval(async () => {
            console.log('监控NBA合约...', foresight.state())
            if (foresight.hasFeederOnline()) {
                const ret = await Eosio.getTableRows(contract.code, contract.scope, contract.table)
                if (ret) {
                    for (const row of ret.rows) {
                        for (const request of row.requests) {
                            if (request.payed && !request.responsed) {
                                payerManager.require(row.payer, request.receipt, request.request_type, request.request_data)
                            }
                        }
                    }
                }
            }
        }, Config.getForesight('nba', 'mspf'))
    }
}
