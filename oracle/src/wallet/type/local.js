
import { Api, JsonRpc } from 'eosjs'
import { JsSignatureProvider } from 'eosjs/dist/eosjs-jssig'
import Fetch from 'node-fetch'
import { TextEncoder, TextDecoder } from 'util'
import Config from '../../config'

const signatureProvider = new JsSignatureProvider(Config.getWallet('local', 'privateKeys'))
const rpc = new JsonRpc(Config.getEosio('network'), { fetch: Fetch })
const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder(), chainId: Config.getEosio('chainId') })

export default {
    async sign(transaction) {
        return await api.transact(transaction, {
            broadcast: false,
            sign: true,
            blocksBehind: 3,
            expireSeconds: 30
        })
        .catch(Config.getHandler('error'))
    }
}
