// https://vuex.vuejs.org/en/actions.html
import firebase from '@/utils/firebase'
import axios from 'axios'

export default {
  fetchNewSensors ({ commit, dispatch }) {
    firebase.database().ref().orderByKey().on("child_added", function(snapshot) {
      if (snapshot.key !== 'StandardArduinoFile'){
        commit('newSensor', { sensorName : snapshot.key })
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
  generateCode ({ commit }, { sensorName } ) {
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
       
        // Clipboard feature disabled
        // if (window) {
        //   window.navigator.clipboard.writeText(scanArray.join(''));
        // }
      },
      (err) => console.error(err)
    )

    return new Promise((resolve, reject) => { 
      // axios.post('http://0.0.0.0:80/ai-generate/', {
      axios.post('https://api.jonascsantos.com/ai-generate/', {
          "scanSamples": JSON.stringify(scanArray),
          "sensorId": sensorName
      }).then(response => {
        resolve(response);  
      }).catch(error => {
        console.error("Error")
        reject(error);
      })
    })
  },
  arduinoCompile ({ commit }) {
    return new Promise((resolve, reject) => { 
      // axios.post('http://0.0.0.0:80/arduino-compile', {
      axios.post('https://api.jonascsantos.com/arduino-compile/', {
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
      // axios.post('http://0.0.0.0:80/binaries-upload', {
      axios.post('https://api.jonascsantos.com/binaries-upload/', {
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
      // axios.post('http://0.0.0.0:80/update-metadata', {
      axios.post('https://api.jonascsantos.com/update-metadata/', {
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
