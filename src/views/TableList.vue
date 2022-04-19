<template>
  <v-container
    fill-height
    fluid
    grid-list-xl
  >
    <v-layout
      justify-center
      wrap
    >
      <v-flex
        md12
      >
        <material-card
          color="purple"
          title="Room Scan"
          text="Enter the room name, hit start, and walk around :D..."
          :loading="true"
        >
          <v-form
            ref="form"
            v-model="valid"
            lazy-validation
          >
            <v-text-field
              v-model="room"
              :counter="20"
              :rules="roomRules"
              label="Room Location"
              :loading="scanStatus"
              :disabled="scanStatus"
              required
            ></v-text-field>
        
            <v-btn
              :disabled="!valid || scanStatus"
              color="success"
              class="mr-4"
              @click="startScan"
            >
              Start
            </v-btn>
            <v-btn
              :disabled="!scanning"
              color="error"
              class="mr-4"
              @click="stopScan"
            >
              Stop
            </v-btn>
          </v-form>
        </material-card>
        <material-card
          color="green"
          title="Device Setup"
          text="Configuration..."
        >
          <div>
            <input type="file" @change="addFile" ref="file">
            <v-btn
              color="blue-grey"
              class="ma-2 white--text"
              @click="uploadFile"
            >
              Upload
              <v-icon
                right
                dark 
              >
                mdi-cloud-upload
              </v-icon>
            </v-btn>
          </div>
        </material-card>
        <material-card
          color="green"
          title="Historical data of User Location"
          text="Data collected every 5 seconds..."
        >
          <v-data-table
            :headers="headers"
            :items="items"
            hide-actions
          >
            <template
              slot="headerCell"
              slot-scope="{ header }"
            >
              <span
                class="subheading font-weight-light text-success text--darken-3"
                v-text="header.text"
              />
            </template>
            <template
              slot="items"
              slot-scope="{ item }"
            >
              <td>{{ timestampToDate(item.Ts) }}</td>
              <td>{{ item.room }}</td>
            </template>
          </v-data-table>
        </material-card>
      </v-flex>
      <v-snackbar
        :color="color"
        :bottom="bottom"
        :top="top"
        :left="left"
        :right="right"
        v-model="snackbar"
        dark
      >
        <v-icon
          color="white"
          class="mr-3"
        >
          mdi-bell-plus
        </v-icon>
        <div>Scan <b>{{scanning ? 'Started' : 'Stopped'}}</b></div>
        <v-icon
          size="16"
          @click="snackbar = false"
        >
          mdi-close-circle
        </v-icon>
      </v-snackbar>
    </v-layout>
  </v-container>
</template>

<script>
import { mapGetters } from 'vuex'
import firebase from '@/utils/firebase'

export default {
  props: {
    sensorId: {
      type: String,
      default: ''
    }
  },

  data: () => ({
    headers: [
      {
        sortable: false,
        text: 'Date/Time',
        value: 'date'
      },
      {
        sortable: false,
        text: 'Room',
        value: 'room'
      }
    ],
    binFile: null,
    valid: false,
    room: '',
    scanning: false,
    roomRules: [
      v => !!v || 'Room name is required'
    ],
    color: null,
    colors: [
      'purple',
      'info',
      'success',
      'warning',
      'error'
    ],
    top: true,
    bottom: false,
    left: false,
    right: false,
    snackbar: false
  }),

  computed: {
    ...mapGetters(['getSensorById']),

    sensor () {
      return this.getSensorById(this.$route.params.sensorId)
    },

    items () {
      return (this.sensor && this.sensor.readings) || []
    },
    scanStatus() {
      if (this.sensor) {
        var dbRef = firebase.database()
          .ref(this.sensor.id +'/SCAN/STATUS')
          .on('value',(snapshot) => {
            const value = snapshot.val();
            if (value && value.running) {
              this.scanning = true;
              this.room = value.room
            } else {
              this.scanning = false;
            }
          },
            (err) => console.error(err)
          )
      }
      return this.scanning;
    }
  },
  methods: {
    timestampToDate( timestamp ) {
      if (!timestamp) {
        return
      }
      
      var a = new Date(timestamp);
      var months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
      var year = a.getFullYear();
      var month = months[a.getMonth()];
      var date = a.getDate();
      var hour = a.getHours();
      var min = a.getMinutes();
      var sec = a.getSeconds();

      var time = date + ' ' + month + ' ' + year + ' ' + hour + ':' + min + ':' + sec ;

      return time;
    },
    startScan () {
      if(this.$refs.form.validate() && !this.scanning) {
        this.scanFirebase(this.room, true); 
      }
    },
    stopScan() {
      if(this.scanning) {
        this.scanFirebase(this.room, false);
      }
    },
    addFile() {
      this.binFile = this.$refs.file.files[0];
    },
    uploadFile() {
      const storageRef = firebase.storage().ref().child("WifiScan");

      let that = this;

      storageRef.put(this.binFile).then(function(snapshot) {
        storageRef.getDownloadURL().then(function(url) {
          that.saveMetadata(that.sensor.id, that.binFile.name, "WifiScan", url);
        })
      });
    },
    scanFirebase( room, running ) {
      var dbRef = firebase.database().ref(this.sensor.id +'/SCAN/STATUS');
      var metadata = {
        running: running,
        room: room
      };
      let that = this;

      dbRef.set(metadata).then(function() {
        that.snack(running ? 'success' : 'error')
      })
      .catch(function(error) {
        console.log("Error: " + error.message);
      });
    },
    saveMetadata( sensorName, filename, version, url) {
      var dbRef = firebase.database().ref(sensorName +'/FIRMWARE/' + version);
      var metadata = {
        filename: filename,
        url: url,
        version: "1"
      };
      dbRef.set(metadata).then(function() {
        window.alert("Success!");
      })
      .catch(function(error) {
        console.log("Error: " + error.message);
      });
    },
    snack (color) {
      this.top = true;

      this.color = color ? color : 'success'

      this.snackbar = true
    }
  }
}
</script>

<style scoped>
  .custom-loader {
    animation: loader 1s infinite;
    display: flex;
  }
  @-moz-keyframes loader {
    from {
      transform: rotate(0);
    }
    to {
      transform: rotate(360deg);
    }
  }
  @-webkit-keyframes loader {
    from {
      transform: rotate(0);
    }
    to {
      transform: rotate(360deg);
    }
  }
  @-o-keyframes loader {
    from {
      transform: rotate(0);
    }
    to {
      transform: rotate(360deg);
    }
  }
  @keyframes loader {
    from {
      transform: rotate(0);
    }
    to {
      transform: rotate(360deg);
    }
  }
</style>
