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
    </v-layout>
  </v-container>
</template>

<script>
import { mapGetters } from 'vuex'

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
    ]
  }),

  computed: {
    ...mapGetters(['getSensorById']),

    sensor () {
      return this.getSensorById(this.$route.params.sensorId)
    },

    items () {
      return (this.sensor && this.sensor.readings) || []
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
