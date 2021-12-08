// https://vuex.vuejs.org/en/actions.html
import firebase from '@/utils/firebase'

export default {
  fetchSensor ({ commit }, sensorName) {
    firebase.database().ref(sensorName)
      .on('value',
        (snapshot) => {
          const value = snapshot.val();
          if (value) {
            commit('receiveSensor', { sensorName, val: value })
          }
        },

        (err) => console.error(err)
      )
  }
}
