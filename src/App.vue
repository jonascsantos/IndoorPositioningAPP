<template>
  <v-app>
    <core-toolbar @input="setFilter"/>

    <core-drawer />

    <core-view />
  </v-app>
</template>

<script>
import { mapMutations, mapActions, mapGetters } from 'vuex'

export default {
  mounted () {
    this.fetchNewSensors()
  },

  methods: {
    ...mapMutations('filter', ['setFilter']),

    ...mapActions(['fetchSensor', 'fetchNewSensors'])
  }
}
</script>

<style lang="scss">
@import '@/styles/index.scss';

/* Remove in 1.2 */
.v-datatable thead th.column.sortable i {
  vertical-align: unset;
}
</style>
