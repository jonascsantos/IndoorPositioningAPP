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
          color="#5EBA93"
          dark
          title="Indoor Positioning System Start-Stop"
          text="Turn On or Off the device tracking"
          :loading="true"
        >
          <v-layout justify-center wrap>
            <v-btn
              :disabled="systemStatus"
              color="success"
              large
              class="mr-4"
              @click="startSystem"
            >
              Start
            </v-btn>
            <v-btn
              :disabled="!indoorPositioningSytem"
              color="error"
              large
              class="mr-4"
              @click="stopSystem"
            >
              Stop
            </v-btn>
          </v-layout>
        </material-card>

        <material-card
          color="#5EBA93"
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
          color="#5EBA93"
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
          color="#5EBA93"
          title="Train Model"
          text="Generate the machine learning converter and classifier."
        >
          <div>
            <v-btn
              :disabled="roomsList.length < 2"
              color="blue-grey"
              class="ma-2 white--text"
              :loading="generateCodeRunning"
              @click="generateCodeButton"
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
          color="#5EBA93"
          title="Device Setup"
          text="Configuration..."
        >
          <div>
            <v-card>
              <v-card-title primary class="title">
                Current Firmware: {{currentFirmware.filename}}
              </v-card-title>
              
              <v-card-text> 
                Board: {{board}}
              </v-card-text>

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
                  Upload .bin File
                  <v-icon
                    right
                    dark 
                  >
                    mdi-cloud-upload
                  </v-icon>
                </v-btn>
              </v-card-actions>
            </v-card>
             <v-card>
              <v-card-title primary class="title">
                Add Arduino File
              </v-card-title>
              
              <v-card-text> 
                <input type="file" @change="addInoFile" ref="file2">
              </v-card-text>
              
              <v-card-actions>
                <v-btn
                  color="blue-grey"
                  class="white--text"
                  @click="uploadInoFile"
                >
                  Upload Ino File
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
          color="#5EBA93"
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
    indoorPositioningSytem: false,
    currentFirmwareFilename: '',
    currentFirmwareVersion: '',
    board: '',
    currentInoFilename: '',
    currentInoVersion: '',
    generateCodeRunning: false,
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
    systemStatus() {
      if (this.sensor) {
        var dbRef = firebase.database()
          .ref(this.sensor.id +'/TRACK_POSITION/STATUS')
          .on('value',(snapshot) => {
            const value = snapshot.val();
            if (value && value.running) {
              this.indoorPositioningSytem = true;
              this.room = value.room
            } else {
              this.indoorPositioningSytem = false;
            }
          },
            (err) => console.error(err)
          )
      }
      return this.indoorPositioningSytem;
    },
    currentFirmware() {
      if (this.sensor) {
        let dbRef = firebase.database()
          .ref(this.sensor.id +'/FIRMWARE/CurrentFirmware')
          .on('value',(snapshot) => {
            const value = snapshot.val();
            if (value && value.version && value.filename) {
              this.currentFirmwareVersion = value.version;
              this.currentFirmwareFilename = value.filename;
            } else {
              this.currentFirmwareVersion = "";
              this.currentFirmwareFilename = ""
            }
          },
            (err) => console.error(err)
          )


        let dbRefBoard = firebase.database()
          .ref(this.sensor.id +'/FIRMWARE/board')
          .on('value',(snapshot) => {
            const value = snapshot.val();
            if (value) {
              this.board = value;
            } else {
              this.board = ""
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
    ...mapActions(['generateCode', 'arduinoCompile', 'binariesUpload', 'updateMetadata']),

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
    startSystem () {
      if(!this.indoorPositioningSytem) {
        this.changeSystemStatus(true); 
      }
    },
    stopSystem() {
      if(this.indoorPositioningSytem) {
        this.changeSystemStatus(false);
      }
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
    async generateCodeButton() {
      this.snack('success')
      this.alert = { 
        text: 'Generating Code..',
        secondaryText: '' 
      }

      this.generateCodeRunning = true;

      const responseGenerateCode = await this.generateCode({
        board:  this.sensor.board,
        sensorName: this.sensor.id
      })
      
      if (responseGenerateCode.status =='200') {
        this.snack('success')
        this.alert = { 
          text: 'Code generated. Compilation started..(2 min)',
          secondaryText: '' 
        }
      }

      const responseCompile = await this.arduinoCompile({
        board: responseGenerateCode.data.board,
        sensorId: responseGenerateCode.data.sensorId
      });

      const responseBinariesUpload = await this.binariesUpload();

      const responseUpdateMetadata = await this.updateMetadata({ 
        url: responseBinariesUpload.data.url, 
        sensorName: responseCompile.data,
        fileName: responseBinariesUpload.data.fileName 
      });

      if (responseUpdateMetadata) {
        this.snack('success')
        this.alert = { 
          text: 'Compilation complete. Uploading firmware..(1 min)',
          secondaryText: '' 
        }

        this.generateCodeRunning = false;
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
          that.saveMetadata('/FIRMWARE/WifiScan', that.sensor.id, that.binFile.name, version , url);
        })
      });
    },
    addInoFile() {
      this.inoFile = this.$refs.file2.files[0];
    },
    uploadInoFile() {
      const storageRef = firebase.storage().ref().child("WifiScanInoFile");

      let that = this;

      let version = this.currentInoVersion && !isNaN(this.currentInoVersion) && 
                    this.currentInoFilename === this.inoFile.name  ? 
                      (+this.currentInoVersion + 1).toString() : "1"

      storageRef.put(this.inoFile).then(function(snapshot) {
        storageRef.getDownloadURL().then(function(url) {
          that.saveMetadata('/FIRMWARE/WifiScanIno',that.sensor.id, that.inoFile.name, version , url);
        })
      });
    },
    changeSystemStatus( running ) {
      var dbRef = firebase.database().ref(this.sensor.id +'/TRACK_POSITION/STATUS');
      var metadata = {
        running: running,
      };
      let that = this;

      dbRef.set(metadata).then(function() {
        that.snack(running ? 'success' : 'error')
        that.alert = { text: "Indoor Positioning System",
                       secondaryText: running ? 'Started' : 'Stopped' }
      })
      .catch(function(error) {
        console.log("Error: " + error.message);
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
    saveMetadata( path, sensorName, filename, version, url) {
      var dbRef = firebase.database().ref(sensorName + path);
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
      var dbRef = firebase.database().ref('/SCAN/data/' + roomName);
      
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
