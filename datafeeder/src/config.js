
const config = {
    oracle: {
        ip:   '127.0.0.1',
        path: '/foresight/nba',
        port: 5002
    }
}

export default {

    getOracleServer() {
        return `http://${config.oracle.ip}:${config.oracle.port}${config.oracle.path}`
    }

}
