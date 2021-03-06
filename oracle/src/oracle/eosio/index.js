
import { Api, JsonRpc } from 'eosjs'
import Fetch from 'node-fetch'
import Config from '../../config'
import { TextEncoder, TextDecoder } from 'util'
import * as Serialize from 'eosjs/dist/eosjs-serialize';

const rpc = new JsonRpc(Config.getEosio('network'), { fetch: Fetch })
const api = new Api({ rpc, signatureProvider: null, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() })

export default {
    async getTableRows(code, scope, table, params = { limit: 10, lower_bound: '', upper_bound: '', index_position: 1 }) {
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
        .catch(Config.getHandler('trycatch'))
    },

    async pushTransaction( tx ) {
        return await rpc.push_transaction({
            signatures:            tx.signatures,
            serializedTransaction: tx.serializedTransaction
        })
        .catch(Config.getHandler('trycatch'))
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
