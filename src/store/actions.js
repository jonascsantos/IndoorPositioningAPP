// https://vuex.vuejs.org/en/actions.html
import firebase from '@/utils/firebase'
import axios from 'axios'

export default {
  fetchNewSensors ({ commit, dispatch }) {
    firebase.database().ref().orderByKey().on("child_added", function(snapshot) {
      const value = snapshot.val();

      if (snapshot.key !== 'StandardArduinoFile' && snapshot.key !== 'SCAN' ){
        commit('newSensor', { sensorName : snapshot.key, board : value.FIRMWARE.board })
        dispatch('fetchSensor', snapshot.key);
      }
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
    
    firebase.database().ref("/SCAN/data")
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

    firebase.database().ref("/SCAN/data")
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
    firebase.database().ref("/SCAN/data/" + roomName)
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
  generateCode ({ commit, getters }, { board, sensorName } ) {
    let scanArray = []

    const sensorsList = getters.sensorsAsArray

    sensorsList.forEach(sensor => {
      firebase.database().ref("/SCAN/data")
      .once('value',
        (snapshot) => {
          snapshot.forEach(function(childSnapshot) {
            const childData = childSnapshot.val();
            let dataArray = _.map(childData)
            dataArray = dataArray.map(({ data: data, ...rest }) => ({ data, ...rest })).map(data => (data.data))
            scanArray = scanArray.concat(dataArray) 
          })
        },
        (err) => console.error(err)
      )
    });

    return new Promise((resolve, reject) => { 
      const url = process.env.NODE_ENV === 'development' ? 'http://0.0.0.0:80/ai-generate/' : 'https://api.jonascsantos.com/ai-generate/'
      axios.post(url, {
          "scanSamples": JSON.stringify(scanArray),
          "sensorId": sensorName,
          "board": board
      }).then(response => {
        resolve(response);  
      }).catch(error => {
        console.error("Error")
        reject(error);
      })
    })
  },
  arduinoCompile ({ commit }, { board, sensorId }) {
    return new Promise((resolve, reject) => { 
      const url = process.env.NODE_ENV === 'development' ? 'http://0.0.0.0:80/arduino-compile/' : 'https://api.jonascsantos.com/arduino-compile/'
      axios.post(url, {
        "boardName": board,
        "sensorId": sensorId
      }).then(response => {
        resolve(response);  
      }).catch(error => {
        console.error("Error")
        reject(error);
      })
    })
  },
  binariesUpload ({ commit }) {
    return new Promise((resolve, reject) => { 
      const url = process.env.NODE_ENV === 'development' ? 'http://0.0.0.0:80/binaries-upload/' : 'https://api.jonascsantos.com/binaries-upload/'
      axios.post(url, {
      }).then(response => {
        resolve(response);  
      }).catch(error => {
        console.error("Error")
        reject(error);
      })
    })
  },
  updateMetadata ({ commit }, { url, sensorName, fileName }) {
    return new Promise((resolve, reject) => { 
      const postUrl = process.env.NODE_ENV === 'development' ? 'http://0.0.0.0:80/update-metadata/' : 'https://api.jonascsantos.com/update-metadata/'
      axios.post(postUrl, {
        "url": url,
        "sensorId": sensorName,
        "fileName": fileName
      }).then(response => {
        resolve(response);  
      }).catch(error => {
        console.error("Error", error)
        reject(error);
      })
    })
  },
}
