
import { Api, JsonRpc } from 'eosjs'
import Fetch from 'node-fetch'
import { getEosio, getHandler } from '../../config'
import { TextEncoder, TextDecoder } from 'util'
import * as Serialize from './eosjs-serialize';

const rpc = new JsonRpc(getEosio('network'), { Fetch })
const api = new Api({ rpc, signatureProvider: null, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() })

export default {
    async getTableRows(code, scope, table, params = { limit: 10, lower_bound: -1, upper_bound: -1, index_position: 0 }) {
        return await rpc.get_table_rows({
            json:           true,
            code:           code,
            scope:          scope,
            table:          table,
            limit:          params.limit,
            lower_bound:    params.lower_bound,
            upper_bound:    params.upper_bound,
            index_position: params.index_position
        })
        .catch(getHandler('error'))
    },

    async pushTransaction(signatures, jsonTransaction) {
        const serializedTransaction = api.serializeTransaction(jsonTransaction)
        return await rpc.push_transaction({
            signatures:            signatures,
            serializedTransaction: serializedTransaction
        })
        .catch(getHandler('error'))
    },

    generateAbiTypes( abi ) {
        return Serialize.getTypesFromAbi(Serialize.createInitialTypes(), abi)
    },

    serializeByAbiType( abiTypes, type, unpackedValue ) {
        const buffer = new Serialize.SerialBuffer({ textEncoder: api.textEncoder, textDecoder: api.textDecoder })
        abiTypes.get(type).serialize(buffer, unpackedValue)
        return buffer.asUint8Array()
    },

    deserializeByAbiType( abiTypes, type, packedValue ) {
        const buffer = new Serialize.SerialBuffer({ textEncoder: api.textEncoder, textDecoder: api.textDecoder })
        buffer.pushArray(packedValue)
        return abiTypes.get(type).deserialize(buffer)
    }
}
