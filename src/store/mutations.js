// https://vuex.vuejs.org/en/mutations.html
import _ from 'lodash'
import Vue from 'vue'

export default {
  receiveSensor (state, { sensorName, val: { ROOM } }) {
    const dataArray = _.map(ROOM).map(({ room: room, ...rest }) => ({ room, ...rest }))
    state.sensors[sensorName] = {
      ...state.sensors[sensorName],
      readings: dataArray
    }
  },
  receiveScannedRoomData (state, payload ) {
    const dataArray = _.map(payload.val).map(({ data: data, ...rest }) => ({ data, ...rest }))
    state.rooms[payload.roomName] = {
      ...state.rooms[payload.roomName],
      readings: dataArray
    }
  },
  deleteScannedRoomData (state, payload ) {
    Vue.delete(state.rooms, payload.roomName)
  },
  newSensor (state, { sensorName }) {
    Vue.set(state.sensors, sensorName, {
      id: sensorName,
      name: 'Device ' + sensorName,
      position: {lat:-9.458305, lng:-40.489484},
      readings: [],
      status: 0
    });
  },
  newScannedRoom (state, { sensorName, roomName }) {
    Vue.set(state.rooms, roomName, {
      id: sensorName + "_" + roomName,
      name: roomName,
      position: {lat:-9.458305, lng:-40.489484},
      readings: [],
      status: 0
    });
  }
}
