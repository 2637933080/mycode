<template>
  <div class="login-container">
    <h1>欢迎使用台风预测系统</h1>
    <h2>用户登录</h2>
    <div class="form-group">
      <label for="username">用户名</label>
      <input 
        type="text" 
        id="username" 
        v-model="form.username" 
        placeholder="请输入用户名"
      />
    </div>
    
    <div class="form-group">
      <label for="password">密码</label>
      <input 
        type="password" 
        id="password" 
        v-model="form.password" 
        placeholder="请输入密码"
      />
    </div>
    
    <div class="error-message" v-if="error">{{ error }}</div>
    
    <div class="form-actions">
      <button @click="login" :disabled="isLoading">
        {{ isLoading ? '登录中...' : '登录' }}
      </button>
      <button @click="switchToRegister" class="secondary">
        注册账号
      </button>
    </div>
  </div>
</template>

<script>
import { reactive, ref } from 'vue';

export default {
  name: 'LoginForm',
  emits: ['login-success', 'switch-to-register'],
  
  setup(props, { emit }) {
    const form = reactive({
      username: '',
      password: ''
    });

    const error = ref('');
    const isLoading = ref(false);

    const login = async () => {
      // 表单验证
      if (!form.username || !form.password) {
        error.value = '用户名和密码不能为空';
        return;
      }
      
      error.value = '';
      isLoading.value = true;
      
      try {
        const response = await fetch('http://127.0.0.1:8010/auth/login', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            username: form.username,
            password: form.password
          })
        });
        
        const data = await response.json();
        
        if (!response.ok) {
          throw new Error(data.message || '登录失败');
        }
        
        // 保存用户信息和token到本地存储
        localStorage.setItem('token', data.token);
        localStorage.setItem('user', JSON.stringify(data.user));
        
        // 通知父组件登录成功
        emit('login-success', data.user);
        
      } catch (err) {
        error.value = err.message || '登录时发生错误';
      } finally {
        isLoading.value = false;
      }
    };

    const switchToRegister = () => {
      emit('switch-to-register');
    };

    return {
      form,
      error,
      isLoading,
      login,
      switchToRegister
    };
  }
}
</script>

<style scoped>
html, body {
    height: 100%;
    margin: 0;
    padding: 0;
}

.login-container {
    max-width: 800px;
    margin: 0 auto;
    padding: 20px;
    border: 1px solid #ddd;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    background-color: white;
}


.form-group {
  margin-bottom: 15px;
}

label {
  display: block;
  margin-bottom: 5px;
  font-weight: bold;
  text-align: left;
}

input {
  width: 100%;
  padding: 10px;
  border: 1px solid #ddd;
  border-radius: 4px;
  font-size: 14px;
  box-sizing: border-box;
}

.form-actions {
  display: flex;
  gap: 10px;
  margin-top: 20px;
}

button {
  padding: 10px 15px;
  border: none;
  border-radius: 4px;
  background-color: #42b983;
  color: white;
  cursor: pointer;
  font-size: 14px;
  flex: 1;
}

button:disabled {
  background-color: #a8d5c2;
  cursor: not-allowed;
}

button.secondary {
  background-color: #e7e7e7;
  color: #333;
}

.error-message {
  color: #ff4c4c;
  margin: 10px 0;
  font-size: 14px;
}
</style>