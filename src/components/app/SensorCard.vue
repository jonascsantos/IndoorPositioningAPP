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
        v-if="room !== null"
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

      <v-layout
        v-if="room === null"
        row
        justify-space-around
        class="pl-3 pr-3"
      >
        <v-progress-linear indeterminate />
      </v-layout>
    </v-card-text>
    <v-card-actions class="pb-5">
      <span
        v-if="lastTimestamp"
      >
        Last reading at {{ timestampToDate(lastTimestamp) }}
      </span>
      <span v-else />
    </v-card-actions>
  </v-card>
</template>

<script>
import _ from 'lodash'

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
  computed: {
    icon () {
      return 'mdi-map-marker'
    },

    room () {
      return this.readings.length ? _.last(this.readings).room : null
    },

    lastTimestamp () {
      return this.readings.length ? _.last(this.readings).Ts : null
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
    }
  }
}
</script>
