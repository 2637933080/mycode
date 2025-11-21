<template>
  <div style="max-width:420px;margin:20px auto">
    <h2>登录</h2>
    <form @submit.prevent="doLogin">
      <div>
        <label>用户名</label>
        <input v-model="username" />
      </div>
      <div>
        <label>密码</label>
        <input type="password" v-model="password" />
      </div>
      <div style="margin-top:12px">
        <button type="submit">登录</button>
      </div>
    </form>
    <p v-if="error" style="color:red">{{ error }}</p>
  </div>
</template>

<script>
export default {
  data() {
    return { username: '', password: '', error: '' }
  },
  methods: {
    async doLogin() {
      this.error = ''
      try {
        const rsp = await fetch('http://127.0.0.1:8010/api/login', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ username: this.username, password: this.password })
        })
        const data = await rsp.json()
        if (!rsp.ok) { this.error = data.error || '登录失败'; return }
        localStorage.setItem('token', data.token)
        localStorage.setItem('username', data.username)
        this.$router.push('/')
      } catch (e) { this.error = e.message }
    }
  }
}
</script>

<style scoped>
label { display:block; margin-top:8px }
input { width:100%; padding:6px }
button { padding:6px 12px }
</style>