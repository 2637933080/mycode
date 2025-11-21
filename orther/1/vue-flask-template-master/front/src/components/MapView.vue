<template>
  <div style="height:calc(100vh - 56px); display:flex">
    <div id="map" style="flex:1"></div>
    <div style="width:320px;padding:8px;border-left:1px solid #ddd;background:#fafafa;overflow:auto">
      <h3>操作</h3>
      <p>点击地图任意位置添加收藏（会提示名称）。</p>
      <p>导航：点击地图上已有点可设为起点/终点，点击下面的按钮绘制直线并显示距离（示例导航）。</p>
      <div style="margin-top:12px">
        <button @click="refreshFavs">刷新收藏</button>
        <button @click="clearRoute">清除导航</button>
      </div>
      <h4 style="margin-top:12px">我的收藏</h4>
      <ul>
        <li v-for="f in favorites" :key="f.id">
          <a href="#" @click.prevent="flyTo(f)">{{ f.name }} ({{ f.lat.toFixed(4) }},{{ f.lng.toFixed(4) }})</a>
          <button @click="removeFav(f.id)" style="margin-left:8px">删除</button>
        </li>
      </ul>
    </div>
  </div>
</template>

<script>
import L from 'leaflet'
import 'leaflet/dist/leaflet.css'

export default {
  data() {
    return { map: null, favorites: [], markers: {}, routeLayer: null, startPoint: null, endPoint: null }
  },
  methods: {
    getAuthHeader() { const t = localStorage.getItem('token'); return t ? { 'Authorization': 'Bearer '+t } : {} },
    async fetchFavs(){
      try{
        const rsp = await fetch('http://127.0.0.1:8010/api/favorites', { headers: this.getAuthHeader() })
        if (!rsp.ok) return this.favorites = []
        const data = await rsp.json()
        this.favorites = data.favorites || []
        this._renderFavorites()
      }catch(e){ console.error(e) }
    },
    async refreshFavs(){ await this.fetchFavs() },
    _renderFavorites(){
      // remove existing
      Object.values(this.markers).forEach(m=>this.map.removeLayer(m))
      this.markers = {}
      this.favorites.forEach(f=>{
        const m = L.marker([f.lat, f.lng]).addTo(this.map).bindPopup(f.name)
        m.on('click', ()=> this._onMarkerClick(f))
        this.markers[f.id] = m
      })
    },
    _onMarkerClick(f){
      if (!this.startPoint) { this.startPoint = f; alert('设置起点: '+f.name); return }
      if (!this.endPoint) { this.endPoint = f; alert('设置终点: '+f.name); this._drawRoute(); return }
      // reset
      this.startPoint = f; this.endPoint = null; alert('重新设置起点: '+f.name)
    },
    _drawRoute(){
      if (!this.startPoint || !this.endPoint) return
      const latlngs = [[this.startPoint.lat, this.startPoint.lng],[this.endPoint.lat, this.endPoint.lng]]
      if (this.routeLayer) this.map.removeLayer(this.routeLayer)
      this.routeLayer = L.polyline(latlngs, {color:'red'}).addTo(this.map)
      const d = this._distance(this.startPoint.lat,this.startPoint.lng,this.endPoint.lat,this.endPoint.lng)
      alert('直线距离: ' + d.toFixed(2) + ' km (示例导航)')
      this.map.fitBounds(this.routeLayer.getBounds(), {padding:[40,40]})
    },
    clearRoute(){ if (this.routeLayer) this.map.removeLayer(this.routeLayer); this.routeLayer=null; this.startPoint=this.endPoint=null },
    _distance(lat1,lon1,lat2,lon2){
      // haversine km
      function toRad(v){return v*Math.PI/180}
      const R=6371
      const dLat=toRad(lat2-lat1), dLon=toRad(lon2-lon1)
      const a=Math.sin(dLat/2)*Math.sin(dLat/2)+Math.cos(toRad(lat1))*Math.cos(toRad(lat2))*Math.sin(dLon/2)*Math.sin(dLon/2)
      const c=2*Math.atan2(Math.sqrt(a),Math.sqrt(1-a))
      return R*c
    },
    async addFavorite(lat,lng){
      const name = prompt('为当前位置输入名称：')
      if (!name) return
      try{
        const rsp = await fetch('http://127.0.0.1:8010/api/favorites', {
          method: 'POST', headers: Object.assign({'Content-Type':'application/json'}, this.getAuthHeader()),
          body: JSON.stringify({ name, lat, lng })
        })
        const data = await rsp.json()
        if (!rsp.ok){ alert(data.error || '添加失败'); return }
        this.fetchFavs()
      }catch(e){ console.error(e) }
    },
    flyTo(f){ this.map.setView([f.lat,f.lng],16); const m = this.markers[f.id]; if(m) m.openPopup() },
    async removeFav(id){
      if(!confirm('确认删除?')) return
      try{
        const rsp = await fetch(`http://127.0.0.1:8010/api/favorites/${id}`, { method:'DELETE', headers:this.getAuthHeader() })
        if (!rsp.ok) { alert('删除失败'); return }
        this.fetchFavs()
      }catch(e){ console.error(e) }
    }
  },
  mounted(){
    // init map
    this.map = L.map('map').setView([39.909,116.397], 12)
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
      maxZoom: 19,
      attribution: '© OpenStreetMap'
    }).addTo(this.map)

    this.map.on('click', (e)=>{
      const { lat, lng } = e.latlng
      // require login
      if (!localStorage.getItem('token')){ if(confirm('未登录，是否前往登录?')) this.$router.push('/login'); return }
      this.addFavorite(lat, lng)
    })

    this.fetchFavs()
    // listen for external jump-to events (from FavoritesList)
    window.addEventListener('jump-to', (e)=>{
      const f = e.detail
      if (f && f.lat && f.lng) this.flyTo(f)
    })
  }
}
</script>

<style scoped>
#map { height: 100%; }
</style>