// https://vuex.vuejs.org/en/mutations.html
import _ from 'lodash'

export default {
  receiveSensor (state, { sensorName, val: { ROOM } }) {
    const flowRateArray = _.map(ROOM).map(({ room: room, ...rest }) => ({ room, ...rest }))
    state.sensors[sensorName] = {
      ...state.sensors[sensorName],
      readings: flowRateArray
    }
  }
}
