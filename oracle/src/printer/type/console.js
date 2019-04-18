
export function error(log) {
    console.error(log)
    return false
}

export function warning(log) {
    console.warn(log)
    return true
}

export function info(log) {
    console.info(log)
    return true
}

export function state(log) {
    console.log('%c ' + log, 'color: green; font-size: 20px')
    return true
}

export function statistics(log) {
    console.log(log)
    return true
}
