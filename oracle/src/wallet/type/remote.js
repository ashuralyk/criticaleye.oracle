
import Http from 'http'
import SockIO from 'socket.io'
import { getWallet } from '../../config'

// 监控器管理器
class WalletManager {
    constructor() {
        this.rejectAddingWallet = false
        this.wallets = []
        this.signatures = []
        this.resolve = null
    }

    addWallet(wallet) {
        if (this.rejectAddingWallet) {
            return false
        } else {
            if (this.wallets.find(wal => wal === wallet)) {
                console.warn('the WalletManager has received a same wallet twice! (ignore)')
                return false
            }
            this.wallets.push(wallet)
            return true
        }
    }

    delWallet(wallet) {
        let i = this.wallets.indexOf(wallet)
        if ( i !== -1 ) {
            this.wallets.splice(i, 1)
        }
        return i
    }

    sign(transaction) {
        if (this.wallets.length > 0) {
            this.wallets.forEach(client => {
                client.emit('sign', transaction)
            })
            this.rejectAddingWallet = true
        } else {
            console.warn(`NO wallet client has been connected <= ${transaction}`)
        }
    }

    receive(signature, wallet) {
        if (this.signatures.find(item => item.wallet === wallet)) {
            console.warn('the WalletManager has received a signature from a same wallet twice! (ignore)')
            return
        }
        this.signatures.push({
            signature: signature,
            wallet: wallet
        })
        if (this.receiveAll()) {
            this.resolve && this.resolve(this.makeSignatureArray())
            this.rejectAddingWallet = false
        }
    }

    setOutsideResolve(resolve) {
        this.resolve = resolve
    }

    receiveAll() {
        let find = true
        for (let wallet of this.wallets) {
            find = this.signatures.find(item => item.wallet === wallet) !== undefined
            if (!find) {
                break
            }
        }
        return find
    }

    makeSignatureArray() {
        return this.signatures.map(item => item.signature)
    }
}

let walletManager = new WalletManager()
let http = null
let io = null

export function startServer() {
    if (http || io) {
        return
    }

    http = Http.createServer()
    io = SockIO(http)

    // 配置钱包连接服务器
    io.of(getWallet('remote', 'path')).on('connection', wallet => {
        console.info(`A wallet(${wallet.conn.remoteAddress}) client's connection has established...`)

        wallet.on('add', publicKey => {
            if (getWallet('remote', 'publicKeys').indexOf(publicKey) > -1) {
                if (walletManager.addWallet(wallet)) {
                    wallet.emit('hello', 'permitted')
                    return
                }
            }
            wallet.emit('hello', 'rejected')
            wallet.disconnect()
        })

        wallet.on('signed', signature => {
            walletManager.receive(signature, wallet)
        })

        wallet.on('disconnect', () => {
            walletManager.delWallet(wallet)
            console.warn(`A wallet(${wallet.conn.remoteAddress}) client's connection has closed yet...`)
        })
    })

    http.listen(getWallet('remote', 'port'))
    console.log('Remote wallet server is running...')
}

// 导出签名接口
export async function sign(transaction) {
    return new Promise(function(resolve, reject) {
        walletManager.setOutsideResolve(resolve)
        walletManager.sign(transaction)
        setTimeout(reject, Config.getWallet('remote', 'timeout'), walletManager.makeSignatureArray())
    })
}
