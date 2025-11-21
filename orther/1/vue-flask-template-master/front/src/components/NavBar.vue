<template>
  <nav class="nav">
    <router-link to="/">地图</router-link>
    <router-link to="/favorites">收藏夹</router-link>
    <span style="flex:1"></span>
    <template v-if="!logged">
      <router-link to="/login">登录</router-link>
      <router-link to="/register">注册</router-link>
    </template>
    <template v-else>
      <a href="#" @click.prevent="logout">登出({{ user }})</a>
    </template>
  </nav>
</template>

<script>
export default {
  data() {
    return {
      user: localStorage.getItem('username') || null
    }
  },
  computed: {
    logged() { return !!localStorage.getItem('token') }
  },
  methods: {
    logout() {
      localStorage.removeItem('token')
      localStorage.removeItem('username')
      this.user = null
      this.$router.push('/login')
      // simple page reload to reset map state
      setTimeout(()=>location.reload(), 200)
    }
  },
  created() {
    this.user = localStorage.getItem('username')
  }
}
</script>

<style scoped>
.nav {
  display:flex;
  gap:12px;
  padding:10px 16px;
  background:#f5f5f5;
  align-items:center;
}
.nav a { color: #333; text-decoration:none }
</style>