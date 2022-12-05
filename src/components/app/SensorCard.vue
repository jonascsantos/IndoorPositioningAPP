<template>
  <v-card>
    <v-card-title
      class="pb-0"
    >
      <div class="headline">
        {{ name }}
      </div>
    </v-card-title>

    <v-card-text>
      <v-layout
        v-if="room !== null && systemStatus"
        row
        justify-space-around
        class="pl-3 pr-3"
      >
        <span
          class="grey--text display-1 font-weight-thin mr-auto"
        >
          {{ room }}

          <v-icon
            color="green"
            large
          >
            {{ icon }}
          </v-icon>
        </span>
      </v-layout>
      <v-layout v-else justify-space-around>
        <span >
          Positioning System is OFF
        </span>
      </v-layout>

      <v-layout
        v-if="!systemStatus"
        row
        justify-space-around
        class="pl-3 pr-3"
      >
            <v-btn
              :disabled="systemStatus"
              color="success"
              large
              @click="startSystem"
            >
              Start
            </v-btn>
      </v-layout>

      <v-layout
        v-if="room === null"
        row
        justify-space-around
        class="pl-3 pr-3"
      >
        <v-progress-linear indeterminate />
      </v-layout>
    </v-card-text>
    <v-card-actions >
      <span
        v-if="lastTimestamp && systemStatus"
        class="pb-5"
      >
        Last reading at {{ timestampToDate(lastTimestamp) }}
      </span>
      <span v-else />
    </v-card-actions>
  </v-card>
</template>

<script>
import _ from 'lodash'
import firebase from '@/utils/firebase'

export default {
  props: {
    id: {
      type: String,
      default: ''
    },

    name: {
      type: String,
      default: ''
    },

    readings: {
      type: Array,
      default: () => []
    }
  },
  data: () => ({
    indoorPositioningSytem: false
  }),
  computed: {
    icon () {
      return 'mdi-map-marker'
    },

    room () {
      return this.readings.length ? _.last(this.readings).room : null
    },

    lastTimestamp () {
      return this.readings.length ? _.last(this.readings).Ts : null
    },
    systemStatus() {
      if (this.id) {
        var dbRef = firebase.database()
          .ref(this.id +'/TRACK_POSITION/STATUS')
          .on('value',(snapshot) => {
            const value = snapshot.val();
            if (value && value.running) {
              this.indoorPositioningSytem = true;
            } else {
              this.indoorPositioningSytem = false;
            }
          },
            (err) => console.error(err)
          )
      }
      return this.indoorPositioningSytem;
    },
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
    changeSystemStatus( running ) {
      var dbRef = firebase.database().ref(this.id +'/TRACK_POSITION/STATUS');
      var metadata = {
        running: running,
      };
      let that = this;

      dbRef.set(metadata).then(function() {
        console.log("System started");
      })
      .catch(function(error) {
        console.log("Error: " + error.message);
      });
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

  }
}
</script>
