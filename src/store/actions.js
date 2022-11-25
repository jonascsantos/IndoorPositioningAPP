// https://vuex.vuejs.org/en/actions.html
import firebase from '@/utils/firebase'
import axios from 'axios'

export default {
  fetchNewSensors ({ commit, dispatch }) {
    firebase.database().ref().orderByKey().on("child_added", function(snapshot) {
      commit('newSensor', { sensorName : snapshot.key })
      dispatch('fetchSensor', snapshot.key);
    });
  },
  fetchSensor ({ commit, dispatch }, sensorName) {
    firebase.database().ref(sensorName)
      .on('value',
        (snapshot) => {
          const value = snapshot.val();
          if (value && value.TRACK_POSITION) {
            commit('receiveSensor', { sensorName, val: value.TRACK_POSITION })
          }
        },

        (err) => console.error(err)
      )
    
    firebase.database().ref(sensorName + "/SCAN/data")
      .on('child_added',
        (snapshot) => {
          const name = snapshot.key;
          if (name) {
            commit('newScannedRoom', { sensorName: sensorName, roomName : name })
            dispatch('fetchScannedRoom', { sensorName: sensorName, roomName : name });
          }
        },
        (err) => console.error(err)
      )

    firebase.database().ref(sensorName + "/SCAN/data")
      .on('child_removed',
        (snapshot) => {
          const name = snapshot.key;
          if (name) {
            commit('deleteScannedRoomData', { roomName : name })
          }
        },
        (err) => console.error(err)
      )
  },
  fetchScannedRoom ({ commit }, { sensorName, roomName} ) {
    firebase.database().ref(sensorName + "/SCAN/data/" + roomName)
      .on('value',
        (snapshot) => {
          const value = snapshot.val();
          if (value) {
            commit('receiveScannedRoomData', { roomName: roomName, val: value })
          }
        },

        (err) => console.error(err)
      )
  },
  outputScanData ({ commit }, { sensorName } ) {
    let scanArray = []
    firebase.database().ref(sensorName + "/SCAN/data")
    .once('value',
      (snapshot) => {
        snapshot.forEach(function(childSnapshot) {
          const childData = childSnapshot.val();
          let dataArray = _.map(childData)
          dataArray = dataArray.map(({ data: data, ...rest }) => ({ data, ...rest })).map(data => (data.data))
          scanArray = scanArray.concat(dataArray) 
        })
        // console.log(scanArray)
        // console.log(scanArray.join(''))

        // axios.post('https://api.jonascsantos.com/items/', {
        axios.post('http://localhost:8500/ai-generate/', {
            "scanSamples": JSON.stringify(scanArray),
        })
        .then(function (response) {
          console.log(response);
        })
        .catch(function (error) {
          console.log(error);
        });

        if (window) {
          window.navigator.clipboard.writeText(scanArray.join(''));
        }
      },
      (err) => console.error(err)
    )

    
  }
}
