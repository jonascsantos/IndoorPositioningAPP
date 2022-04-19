// https://vuex.vuejs.org/en/mutations.html
import _ from 'lodash'
import Vue from 'vue'

export default {
  receiveSensor (state, { sensorName, val: { ROOM } }) {
    const flowRateArray = _.map(ROOM).map(({ room: room, ...rest }) => ({ room, ...rest }))
    state.sensors[sensorName] = {
      ...state.sensors[sensorName],
      readings: flowRateArray
    }
  },
  newSensor (state, { sensorName }) {
    Vue.set(state.sensors, sensorName, {
      id: sensorName,
      name: 'Device ' + sensorName,
      position: {lat:-9.458305, lng:-40.489484},
      readings: [],
      status: 0
    });
  }
}
