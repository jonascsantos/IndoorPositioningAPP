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
          color="pink"
          title="Scanned Rooms"
          text="Data about the scanned rooms"
        >
          <v-container fluid grid-list-md>
            <v-layout row wrap>
              <v-flex 
                v-for="({ id, name, readings }, key) in roomsList"
                :key="key" 
                d-flex xs12 sm6 md4
              >
                <v-card outlined>
                  <v-card-title primary class="title">{{name}}</v-card-title>
                  <v-card-text>Total Scan Samples: <b>{{readings.length}}</b></v-card-text>
                  <v-card-actions>
                    <v-btn
                      color="error"
                      @click="deleteRoom(name)"
                    >
                      Delete Room
                    </v-btn>
                  </v-card-actions>
                </v-card>
              </v-flex>
              
            </v-layout>
          </v-container>
        </material-card>
        <material-card
          color="deep-purple"
          title="Train Model"
          text="Generate the machine learning converter and classifier."
        >
          <div>
            <v-btn
              :disabled="roomsList.length < 2"
              color="blue-grey"
              class="ma-2 white--text"
              @click="generateCode"
            >
              Generate Code
              <v-icon
                right
                dark 
              >
                mdi-auto-fix
              </v-icon>
            </v-btn>
          </div>
        </material-card>
        <material-card
          color="green"
          title="Device Setup"
          text="Configuration..."
        >
          <div>
            <v-card>
              <v-card-title primary class="title">
                Current Firmware: {{currentFirmware.filename}}
              </v-card-title>
              
              <v-card-text> 
                Version {{currentFirmware.version}}
              </v-card-text>
              
              <v-card-text> 
                <input type="file" @change="addFile" ref="file">
              </v-card-text>
              
              <v-card-actions>
                <v-btn
                  color="blue-grey"
                  class="white--text"
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
              </v-card-actions>
            </v-card>
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
        <div>{{alert.text}} <b>{{alert.secondaryText}}</b></div>
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
import { mapGetters, mapState, mapActions } from 'vuex'
import firebase from '@/utils/firebase'
import _ from 'lodash'

const normalize = (str) => str.normalize('NFD').replace(/[\u0300-\u036f]/g, '').toLowerCase()

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
    currentFirmwareFilename: '',
    currentFirmwareVersion: '',
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
    snackbar: false,
    alert: ''
  }),

  computed: {
    ...mapGetters(['getSensorById']),

    ...mapState(['rooms']),

    ...mapState('filter', {
      filter: ({ value }) => value
    }),

    roomsList () {
      return _.filter(this.rooms, ({ name }) => normalize(name).match(new RegExp(normalize(this.filter))))
    },

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
    },
    currentFirmware() {
      if (this.sensor) {
        var dbRef = firebase.database()
          .ref(this.sensor.id +'/FIRMWARE/CurrentFirmware')
          .on('value',(snapshot) => {
            const value = snapshot.val();
            if (value && ((value.filename != this.currentFirmwareFilename) ||
                           value.version != this.currentFirmwareVersion)) {
              this.currentFirmwareVersion = value.version;
              this.currentFirmwareFilename = value.filename;
              
            } 
          },
            (err) => console.error(err)
          )
      }

      let obj = {
        filename: this.currentFirmwareFilename,
        version: this.currentFirmwareVersion
      };

      return obj;
    },
  },
  methods: {
    ...mapActions(['outputScanData']),

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
    generateCode() {
      this.outputScanData({
        sensorName: this.sensor.id
      });
      this.snack('success')
      this.alert = { 
        text: 'Scan data copied to clipboard!',
        secondaryText: '' 
      }
    },
    addFile() {
      this.binFile = this.$refs.file.files[0];
    },
    uploadFile() {
      const storageRef = firebase.storage().ref().child("WifiScan");

      let that = this;

      let version = this.currentFirmwareVersion && !isNaN(this.currentFirmwareVersion) && 
                    this.currentFirmwareFilename === this.binFile.name  ? 
                      (+this.currentFirmwareVersion + 1).toString() : "1"

      storageRef.put(this.binFile).then(function(snapshot) {
        storageRef.getDownloadURL().then(function(url) {
          that.saveMetadata(that.sensor.id, that.binFile.name, version , url);
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
        that.alert = { text: "Scan",
                       secondaryText: running ? 'Started' : 'Stopped' }
      })
      .catch(function(error) {
        console.log("Error: " + error.message);
      });
    },
    saveMetadata( sensorName, filename, version, url) {
      var dbRef = firebase.database().ref(sensorName +'/FIRMWARE/WifiScan');
      var metadata = {
        filename: filename,
        url: url,
        version: version
      };
      dbRef.set(metadata).then(function() {
        window.alert("Success!");
      })
      .catch(function(error) {
        console.log("Error: " + error.message);
      });
    },
    deleteRoom( roomName ) {
      var dbRef = firebase.database().ref(this.sensor.id +'/SCAN/data/' + roomName);
      
      let that = this
      
      dbRef.remove().then(function() {
        that.snack('error')
        that.alert = { text: roomName,
                       secondaryText: 'DELETED' }
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
