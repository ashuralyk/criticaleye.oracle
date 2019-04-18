
import { Api, JsonRpc } from 'eosjs'
import { JsSignatureProvider } from 'eosjs/dist/eosjs-jssig'
import Fetch from 'node-fetch'
import { TextEncoder, TextDecoder } from 'util'
import { getWallet, getEosio, getHandler } from '../../config'

const signatureProvider = new JsSignatureProvider(getWallet('local', 'privateKeys'))
const rpc = new JsonRpc(getEosio('network'), { Fetch })
const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder(), chainId: getEosio('chainId') })

export async function sign(transaction) {
    return await api.transact(transaction, {
        broadcast: false,
        sign: true
    })
    .catch(getHandler('error'))
}
