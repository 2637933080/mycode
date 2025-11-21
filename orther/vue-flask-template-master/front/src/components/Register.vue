<template>
  <div class="register-container">
    <h2>用户注册</h2>
    <div class="form-group">
      <label for="username">用户名</label>
      <input 
        type="text" 
        id="username" 
        v-model="form.username" 
        placeholder="请输入用户名（3-20个字符）"
      />
    </div>
    
    <div class="form-group">
      <label for="email">电子邮箱</label>
      <input 
        type="email" 
        id="email" 
        v-model="form.email" 
        placeholder="请输入有效的电子邮箱"
      />
    </div>
    
    <div class="form-group">
      <label for="password">密码</label>
      <input 
        type="password" 
        id="password" 
        v-model="form.password" 
        placeholder="请输入密码（至少6位）"
      />
    </div>
    
    <div class="form-group">
      <label for="confirmPassword">确认密码</label>
      <input 
        type="password" 
        id="confirmPassword" 
        v-model="form.confirmPassword" 
        placeholder="请再次输入密码"
      />
    </div>
    
    <div class="error-message" v-if="error">{{ error }}</div>
    
    <div class="form-actions">
      <button @click="register" :disabled="isLoading">
        {{ isLoading ? '注册中...' : '注册' }}
      </button>
      <button @click="switchToLogin" class="secondary">
        返回登录
      </button>
    </div>
  </div>
</template>

<script>
import { reactive, ref } from 'vue';

export default {
  name: 'RegisterForm',
  emits: ['register-success', 'switch-to-login'],
  
  setup(props, { emit }) {
    const form = reactive({
      username: '',
      email: '',
      password: '',
      confirmPassword: ''
    });

    const error = ref('');
    const isLoading = ref(false);

    // 验证邮箱格式
    const isValidEmail = (email) => {
      const re = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
      return re.test(email);
    };

    const register = async () => {
      // 表单验证
      if (!form.username || !form.email || !form.password || !form.confirmPassword) {
        error.value = '所有字段都是必填的';
        return;
      }
      
      if (form.username.length < 3 || form.username.length > 20) {
        error.value = '用户名长度必须在3-20个字符之间';
        return;
      }
      
      if (!isValidEmail(form.email)) {
        error.value = '请输入有效的电子邮箱';
        return;
      }
      
      if (form.password.length < 6) {
        error.value = '密码长度至少6位';
        return;
      }
      
      if (form.password !== form.confirmPassword) {
        error.value = '两次输入的密码不一致';
        return;
      }
      
      error.value = '';
      isLoading.value = true;
      
      try {
        const response = await fetch('http://127.0.0.1:8010/auth/register', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            username: form.username,
            email: form.email,
            password: form.password
          })
        });
        
        const data = await response.json();
        
        if (!response.ok) {
          throw new Error(data.message || '注册失败');
        }
        
        // 通知父组件注册成功
        emit('register-success');
        
      } catch (err) {
        error.value = err.message || '注册时发生错误';
      } finally {
        isLoading.value = false;
      }
    };

    const switchToLogin = () => {
      emit('switch-to-login');
    };

    return {
      form,
      error,
      isLoading,
      register,
      switchToLogin
    };
  }
}
</script>

<style scoped>
.register-container {
  max-width: 800px;
  margin: 0 auto;
  padding: 20px;
  border: 1px solid #ddd;
  border-radius: 8px;
  box-shadow: 0 2px 10px rgba(0,0,0,0.1);
  position: relative;
  top: 100px;
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