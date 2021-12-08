<template>
  <div>
    <div class="mapouter">
      <div class="gmap_canvas">
        <gmap-map
          ref="gMap"
          :center="{ lat:-9.457740, lng:-40.489282 }"
          :zoom="18"
          map-type-id="satellite"
          style="width: 100%; height: 100%"
        >
          <template
            v-for="({ position, id }, key) in sensorsAsArray"
          >
            <gmap-marker
              v-if="sensorsAsArray && sensorsAsArray.length && flowById(id)"
              :position="position"
              :icon="flowById(id).flow < 10 || flowById(id).flow > 40 ? redIcon : greenIcon"
              :key="key"
            />
          </template>
        </gmap-map>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'
import greenIcon from '@/icons/check_circle.svg'
import redIcon from '@/icons/error.svg'

export default {
  props: {
    sensorId: {
      type: String,
      default: ''
    }
  },

  data () {
    return {
      zoom: 18,
      greenIcon,
      redIcon
    }
  },

  computed: {
    ...mapGetters(['sensorsAsArray', 'flowById'])
  }
}
</script>

<style>
.mapouter {
  text-align:right;
  height:100%;
  width:100%;
  position: absolute;
}
.gmap_canvas {
  overflow:hidden;
  background:none!important;
  height:100%;
  width:100%;
}
</style>
