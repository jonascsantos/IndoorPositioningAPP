import firebase from 'firebase'
import "firebase/storage";

export const config = {
  apiKey: 'AIzaSyC6h2tKVluciekxZyoOepJNd03jVIYNgKA',
  authDomain: 'flowsensor-bfbed.firebaseapp.com',
  databaseURL: 'https://flowsensor-bfbed.firebaseio.com',
  projectId: 'flowsensor-bfbed',
  storageBucket: 'flowsensor-bfbed.appspot.com',
  messagingSenderId: '529363595938',
  appId: "1:529363595938:web:25060447cf953e866c1dfb"
}

let firebaseApp

if (firebase.apps && firebase.apps.length > 0) {
  firebaseApp = firebase.apps[0]
} else {
  firebaseApp = firebase.initializeApp(config)
}

export default firebaseApp
