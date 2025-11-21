<template>
  <div style="padding:12px;max-width:760px;margin:8px auto">
    <h2>收藏夹</h2>
    <div v-if="!token">
      请先登录后查看收藏。<br/>
      <router-link to="/login">去登录</router-link>
    </div>
    <div v-else>
      <button @click="load">刷新</button>
      <ul>
        <li v-for="f in favorites" :key="f.id">
          <strong>{{ f.name }}</strong> — {{ f.lat.toFixed(4) }}, {{ f.lng.toFixed(4) }}
          <button @click="goTo(f)" style="margin-left:8px">在地图中查看</button>
          <button @click="del(f.id)" style="margin-left:8px">删除</button>
        </li>
      </ul>
    </div>
  </div>
</template>

<script>
export default {
  data(){ return { favorites: [], token: localStorage.getItem('token') } },
  methods:{
    async load(){
      try{
        const rsp = await fetch('http://127.0.0.1:8010/api/favorites', { headers: { 'Authorization': 'Bearer '+this.token } })
        if(!rsp.ok) return this.favorites = []
        const d = await rsp.json(); this.favorites = d.favorites || []
      }catch(e){ console.error(e) }
    },
    goTo(f){ this.$router.push('/'); setTimeout(()=>{ window.dispatchEvent(new CustomEvent('jump-to', { detail: f })) }, 300) },
    async del(id){ if(!confirm('删除?')) return; await fetch(`http://127.0.0.1:8010/api/favorites/${id}`, { method:'DELETE', headers:{ 'Authorization':'Bearer '+this.token } }); this.load() }
  },
  mounted(){ if(this.token) this.load() }
}
</script>

<style scoped>
button { padding:4px 8px }
</style>