
import Local  from './type/local'
import Remote from './type/remote'
import Config from '../config'

export default {
    async sign(transaction) {
        switch (Config.getWallet('use')) {
            case 'local':  return await Local.sign(transaction)
            case 'remote': Remote.startServer(); return await Remote.sign(transaction)
        }
        throw new Error('the "use" of wallet config is incorrect, please check the configruation (must be "local" or "remote")')
    }
}
