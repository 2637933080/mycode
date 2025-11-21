<template>
  <div style="max-width:420px;margin:20px auto">
    <h2>注册</h2>
    <form @submit.prevent="doRegister">
      <div>
        <label>用户名</label>
        <input v-model="username" />
      </div>
      <div>
        <label>密码</label>
        <input type="password" v-model="password" />
      </div>
      <div style="margin-top:12px">
        <button type="submit">注册</button>
      </div>
    </form>
    <p v-if="error" style="color:red">{{ error }}</p>
    <p v-if="ok" style="color:green">注册成功，请<a @click.prevent="$router.push('/login')" href="#">登录</a></p>
  </div>
</template>

<script>
export default {
  data() { return { username: '', password: '', error: '', ok: false } },
  methods: {
    async doRegister() {
      this.error = ''
      try {
        const rsp = await fetch('http://127.0.0.1:8010/api/register', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ username: this.username, password: this.password })
        })
        const data = await rsp.json()
        if (!rsp.ok) { this.error = data.error || '注册失败'; return }
        this.ok = true
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