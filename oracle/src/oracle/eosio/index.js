
import { JsonRpc } from 'eosjs'
import Fetch from 'node-fetch'
import { getEosio, getHandler } from '../../config'

const rpc = new JsonRpc(getEosio('network'), { Fetch })

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

    async pushTransaction(signatures, serializedTransaction) {
        return await rpc.push_transaction({
            signatures:            signatures,
            serializedTransaction: serializedTransaction
        })
        .catch(getHandler('error'))
    }
}
