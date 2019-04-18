
import * as NBA from './foresight/nba/nba'
import * as FIFA from './foresight/fifa/fifa'
import { getForesight } from '../config'

export default function start() {
    getForesight('use').forEach(oracle => {
        switch (oracle) {
            case 'nba':  NBA.start(); break
            case 'fifa': FIFA.start(); break
        }
    })
}
