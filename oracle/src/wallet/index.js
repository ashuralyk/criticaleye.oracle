
import * as Local from './type/local'
import * as Remote from './type/remote'
import { getWallet } from '../config'

export default async function sign(transaction) {
    switch (getWallet('use')) {
        case 'local':  return await Local.sign(transaction)
        case 'remote': Remote.startServer(); return await Remote.sign(transaction)
    }
    throw new Error('the "use" of wallet config is incorrect, please check the configruation (must be "local" or "remote")')
}
