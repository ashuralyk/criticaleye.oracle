
import NBA from './foresight/nba/nba'
import FIFA from './foresight/fifa/fifa'
import Config from '../config'

export default {
     start() {
        Config.getForesight('use').forEach(oracle => {
            switch (oracle) {
                case 'nba':  NBA.start(); break
                case 'fifa': FIFA.start(); break
            }
        })
    }
}
