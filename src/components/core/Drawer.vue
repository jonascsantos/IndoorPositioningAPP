<template>
  <v-navigation-drawer
    id="app-drawer"
    v-model="inputValue"
    app
    dark
    floating
    persistent
    mobile-break-point="991"
    width="260"
  >
    <v-layout
      class="fill-height"
      tag="v-list"
      column
    >
      <v-list-tile avatar>
        <v-list-tile-avatar
          color="white"
        >
          <v-avatar
            height="34"
            contain
          >
            <img
              :src="logo"
              alt="Indoor Positioning"
            >
          </v-avatar>

        </v-list-tile-avatar>
        <v-list-tile-title class="title">
          Indoor Positioning
        </v-list-tile-title>
      </v-list-tile>
      <v-divider/>
      <v-list-tile
        v-for="(link, i) in links"
        :key="i"
        :to="link.to"
        :active-class="color"
        avatar
        class="v-list-item"
      >
        <v-list-tile-action>
          <v-icon>{{ link.icon }}</v-icon>
        </v-list-tile-action>
        <v-list-tile-title
          v-text="link.text"
        />
      </v-list-tile>
    </v-layout>
  </v-navigation-drawer>
</template>

<script>
// Utilities
import {  mapMutations, mapState} from 'vuex'
import _ from 'lodash'

const normalize = (str) => str.normalize('NFD').replace(/[\u0300-\u036f]/g, '').toLowerCase()

export default {
  data: () => ({
    logo: './img/logo.jpg',
    responsive: false
  }),
  computed: {
    ...mapState('app', ['image', 'color']),

    ...mapState(['sensors']),

    ...mapState('filter', {
      filter: ({ value }) => value
    }),

    sensorList () {
      return _.filter(this.sensors, ({ name }) => normalize(name).match(new RegExp(normalize(this.filter))))
    },
    inputValue: {
      get () {
        return this.$store.state.app.drawer
      },
      set (val) {
        this.setDrawer(val)
      }
    },
    links() {
      let linksArray = [
        {
          to: '/dashboard',
          icon: 'mdi-view-dashboard',
          text: 'Dashboard'
        },
        // {
        //   to: '/maps',
        //   icon: 'mdi-map-marker',
        //   text: 'Map'
        // }
      ]

      this.sensorList.forEach(sensor => {
        linksArray.push({
          to:'/table-list/' + sensor.id,
          icon: 'mdi-account-search',
          text: 'Device ' + sensor.id
        })
      });

      return linksArray
    },
    items () {
      return this.$t('Layout.View.items')
    }
  },
  mounted () {
    this.onResponsiveInverted()
    window.addEventListener('resize', this.onResponsiveInverted)
  },
  beforeDestroy () {
    window.removeEventListener('resize', this.onResponsiveInverted)
  },
  methods: {
    ...mapMutations('app', ['setDrawer', 'toggleDrawer']),
    onResponsiveInverted () {
      if (window.innerWidth < 991) {
        this.responsive = true
      } else {
        this.responsive = false
      }
    }
  }
}
</script>

<style lang="scss">
  #app-drawer {
    .v-list__tile {
      border-radius: 4px;

      &--buy {
        margin-top: auto;
        margin-bottom: 17px;
      }
    }

    .v-image__image--contain {
      top: 9px;
      height: 60%;
    }

    .search-input {
      margin-bottom: 30px !important;
      padding-left: 15px;
      padding-right: 15px;
    }
  }
</style>
