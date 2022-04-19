// https://vuex.vuejs.org/en/actions.html
import firebase from '@/utils/firebase'

export default {
  fetchNewSensors ({ commit, dispatch }) {
    firebase.database().ref().orderByKey().on("child_added", function(snapshot) {
      commit('newSensor', { sensorName : snapshot.key })
      dispatch('fetchSensor', snapshot.key);
    });
  },
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
