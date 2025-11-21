<template>
  <div class="layout-container" v-if="currentUser">
    <div class="user-profile">
      <p>{{ currentUser.username }}</p>
      <button class="logout-btn" @click="logout">退出</button>
    </div>
    
    <!-- 台风选择器 -->
    <div class="typhoon-selector">
      <div class="selector-container">
        <select v-model="selectedSample" @change="handleSampleChange">
          <option value="">请选择台风样本</option>
          <option v-for="sample in typhoonSamples" :key="sample.id" :value="sample.id">
            {{ sample.name }}
          </option>
        </select>
        <button @click="showTyphoonPath" :disabled="!selectedSample" class="show-path-btn">
          {{ isLoading ? '加载中...' : '显示台风路径' }}
        </button>
      </div>
      
      <div class="typhoon-info" v-if="currentTyphoonData">
        <div class="legend">
          <div class="legend-item">
            <div class="path-color history"></div>
            <span>历史路径</span>
          </div>
          <div class="legend-item">
            <div class="path-color prediction"></div>
            <span>预测路径</span>
          </div>
          <div class="legend-item">
            <div class="path-color actual"></div>
            <span>实际路径</span>
          </div>
        </div>
        <!-- 已移除 "显示的是台风样本 X 的历史、预测和实际路径" 文字 -->
      </div>
    </div>
    
    <!-- 台风数据调试面板 -->
    <div class="debug-panel">
      <h3>调试面板 <span class="toggle-btn" @click="toggleDebugPanel">[{{ showDebugPanel ? '隐藏' : '显示' }}]</span></h3>
      
      <div v-if="showDebugPanel">
        <div class="debug-actions">
          <button @click="fetchTyphoonCount" class="debug-btn">获取数据统计</button>
          <button @click="fetchDataPaths" class="debug-btn">检查数据路径</button>
          <button @click="reloadTyphoonData" class="debug-btn warning">重新加载数据</button>
        </div>
        
        <div v-if="debugLoading" class="debug-loading">正在加载数据...</div>
        
        <div class="debug-info" v-if="debugInfo">
          <h4>数据统计信息</h4>
          <pre>{{ JSON.stringify(debugInfo, null, 2) }}</pre>
        </div>
        
        <div class="debug-info" v-if="debugPaths">
          <h4>数据文件路径信息</h4>
          <pre>{{ JSON.stringify(debugPaths, null, 2) }}</pre>
        </div>
        
        <div v-if="sampleDebugData" class="debug-info">
          <h4>样本 {{ selectedSample }} 的调试数据</h4>
          <div class="sample-data">
            <p>共有 {{ sampleDebugData.count }} 条记录</p>
            <button @click="viewSampleDetails = !viewSampleDetails" class="debug-btn small">
              {{ viewSampleDetails ? '隐藏详情' : '查看详情' }}
            </button>
            <pre v-if="viewSampleDetails">{{ JSON.stringify(sampleDebugData.data, null, 2) }}</pre>
          </div>
        </div>
      </div>
    </div>
    
    <!-- 高德地图容器 -->
    <div id="map-container" class="map-container"></div>
  </div>
  
  <div v-else>
    <h1>{{ msg }}</h1>
    <div class="auth-section">
      <div v-if="currentView === 'login'">
        <Login 
          @login-success="handleLoginSuccess" 
          @switch-to-register="currentView = 'register'" 
        />
      </div>
      <div v-else>
        <Register
          @register-success="handleRegisterSuccess"
          @switch-to-login="currentView = 'login'"
        />
      </div>
    </div>
  </div>
</template>

<script>
import { reactive, onMounted, ref, nextTick } from 'vue'
import Login from './Login.vue'
import Register from './Register.vue'

export default {
  name: 'HelloWorld',
  components: {
    Login,
    Register
  },
  props: {
    msg: String
  },
  setup() {
    const currentView = ref('login')
    const currentUser = ref(null)
    const apiStatus = ref(true)
    const map = ref(null)
    const typhoonSamples = ref([])
    const selectedSample = ref('')
    const currentTyphoonData = ref(null)
    const isLoading = ref(false)
    const historyPath = ref(null)
    const predictionPath = ref(null)
    const actualPath = ref(null)
    const historyMarkers = ref([])
    const predictionMarkers = ref([])
    const actualMarkers = ref([])

    // 调试相关状态
    const showDebugPanel = ref(false) // 修改：调试面板默认折叠
    const debugLoading = ref(false)
    const debugInfo = ref(null)
    const debugPaths = ref(null)
    const sampleDebugData = ref(null)
    const viewSampleDetails = ref(false)

    // 检查是否已登录
    const checkAuth = () => {
      const userStr = localStorage.getItem('user')
      if (userStr) {
        try {
          currentUser.value = JSON.parse(userStr)
        } catch (e) {
          localStorage.removeItem('user')
          localStorage.removeItem('token')
        }
      }
    }

    // 登录成功处理
    const handleLoginSuccess = (user) => {
      currentUser.value = user
      // 登录成功后初始化地图
      nextTick(() => {
        initMap()
        fetchTyphoonSamples()
      })
    }

    // 注册成功处理
    const handleRegisterSuccess = () => {
      currentView.value = 'login'
      // 可以显示一个成功注册的提示
      alert('注册成功！请登录')
    }

    // 退出登录
    const logout = () => {
      localStorage.removeItem('user')
      localStorage.removeItem('token')
      currentUser.value = null
      currentView.value = 'login'
    }

    // 初始化高德地图
    const initMap = () => {
      // 确保地图容器已经渲染
      const mapContainer = document.getElementById('map-container')
      if (!mapContainer) return

      // 设置安全密钥
      window._AMapSecurityConfig = {
        securityJsCode: '20acd37a8d130884a6ddf67a502c0010',
      }

      // 创建地图实例
      map.value = new AMap.Map('map-container', {
        zoom: 4, // 初始缩放级别
        center: [135.0, 12.0], // 初始中心位置（根据台风数据大致位置设置）
        viewMode: '3D', // 使用3D视图
        resizeEnable: true // 自动调整大小
      })

      // 添加比例尺控件 - 修复高德地图2.0版本API变化
      AMap.plugin(['AMap.Scale', 'AMap.ToolBar', 'AMap.HawkEye'], function() {
        // 添加比例尺
        map.value.addControl(new AMap.Scale({
          position: 'LB' // 左下角
        }));
        
        // 添加工具条，可以实现地图的缩放、平移等操作
        map.value.addControl(new AMap.ToolBar({
          position: 'RB' // 右下角
        }));
        
        // 添加鹰眼控件
        map.value.addControl(new AMap.HawkEye({
          opened: false // 默认不展开
        }));
      });
    }

    // 获取台风样本列表
    const fetchTyphoonSamples = async () => {
      try {
        const response = await fetch('http://127.0.0.1:8010/api/typhoon/samples')
        const data = await response.json()
        typhoonSamples.value = data
      } catch (error) {
        console.error('获取台风样本列表失败:', error)
        alert('获取台风样本列表失败')
      }
    }

    // 切换台风样本
    const handleSampleChange = () => {
      // 清除当前显示的路径
      clearTyphoonPaths()
      currentTyphoonData.value = null
    }

    // 显示台风路径
    const showTyphoonPath = async () => {
      if (!selectedSample.value) return
      
      isLoading.value = true
      clearTyphoonPaths()
      
      try {
        // 请求台风数据
        const response = await fetch(`http://127.0.0.1:8010/api/typhoon/data/${selectedSample.value}`)
        const data = await response.json()
        currentTyphoonData.value = data
        
        // 绘制台风路径
        drawTyphoonPaths()
        
        // 调整地图视野，确保路径完全可见
        adjustMapView()
      } catch (error) {
        console.error('获取台风数据失败:', error)
        alert('获取台风数据失败')
      } finally {
        isLoading.value = false
      }
    }

    // 清除台风路径
    const clearTyphoonPaths = () => {
      // 清除历史路径
      if (historyPath.value) {
        map.value.remove(historyPath.value)
        historyPath.value = null
      }
      
      // 清除预测路径
      if (predictionPath.value) {
        map.value.remove(predictionPath.value)
        predictionPath.value = null
      }
      
      // 清除实际路径
      if (actualPath.value) {
        map.value.remove(actualPath.value)
        actualPath.value = null
      }
      
      // 清除历史路径标记点
      if (historyMarkers.value && historyMarkers.value.length > 0) {
        map.value.remove(historyMarkers.value)
        historyMarkers.value = []
      }
      
      // 清除预测路径标记点
      if (predictionMarkers.value && predictionMarkers.value.length > 0) {
        map.value.remove(predictionMarkers.value)
        predictionMarkers.value = []
      }
      
      // 清除实际路径标记点
      if (actualMarkers.value && actualMarkers.value.length > 0) {
        map.value.remove(actualMarkers.value)
        actualMarkers.value = []
      }
    }

    // 绘制台风路径
    const drawTyphoonPaths = () => {
      if (!currentTyphoonData.value || !map.value) return
      
      const { paths } = currentTyphoonData.value
      
      // 清除之前的路径
      clearTyphoonPaths()
      
      // 绘制历史轨迹 (绿色)
      if (paths.history && paths.history.length > 0) {
        const historyPoints = paths.history.map(point => [point.longitude, point.latitude])
        historyPath.value = new AMap.Polyline({
          path: historyPoints,
          strokeColor: '#32CD32', // 绿色线
          strokeWeight: 3,
          strokeOpacity: 0.8,
          strokeStyle: 'solid',
          lineJoin: 'round'
        })
        map.value.add([historyPath.value])
        
        // 添加历史轨迹标记点
        addMarkers(paths.history, 'history', '#32CD32', historyMarkers)
      }
      
      // 绘制预测轨迹 (蓝色)
      if (paths.prediction && paths.prediction.length > 0) {
        const predictionPoints = paths.prediction.map(point => [point.longitude, point.latitude])
        predictionPath.value = new AMap.Polyline({
          path: predictionPoints,
          strokeColor: '#1874CD', // 蓝色线
          strokeWeight: 3,
          strokeOpacity: 0.8,
          strokeStyle: 'solid',
          lineJoin: 'round'
        })
        map.value.add([predictionPath.value])
        
        // 添加预测轨迹标记点
        addMarkers(paths.prediction, 'prediction', '#1874CD', predictionMarkers)
      }
      
      // 绘制实际轨迹 (红色)
      if (paths.actual && paths.actual.length > 0) {
        const actualPoints = paths.actual.map(point => [point.longitude, point.latitude])
        actualPath.value = new AMap.Polyline({
          path: actualPoints,
          strokeColor: '#FF3030', // 红色线
          strokeWeight: 3,
          strokeOpacity: 0.8,
          strokeStyle: 'solid',
          lineJoin: 'round'
        })
        map.value.add([actualPath.value])
        
        // 添加实际轨迹标记点
        addMarkers(paths.actual, 'actual', '#FF3030', actualMarkers)
      }
    }

    // 添加标记点辅助函数
    const addMarkers = (points, trackType, color, markersArray) => {
      points.forEach((point) => {
        // 创建标记点，使用台风等级而不是步骤编号
        const marker = new AMap.Marker({
          position: [point.longitude, point.latitude],
          title: `${trackType === 'history' ? '历史' : trackType === 'prediction' ? '预测' : '实际'}路径点 (等级: ${point.level.toFixed(1)})`,
          label: {
            content: `${point.level.toFixed(1)}`,  // 显示台风等级而不是步骤编号
            direction: 'center',
            offset: new AMap.Pixel(0, 0)
          },
          icon: new AMap.Icon({
            size: new AMap.Size(20, 20),
            image: `data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20"><circle cx="10" cy="10" r="8" fill="${color}" stroke="white" stroke-width="1"/></svg>`,
            imageSize: new AMap.Size(20, 20)
          })
        })
        
        // 添加点击事件显示详细信息
        marker.on('click', () => {
          // 创建信息窗体
          const infoWindow = new AMap.InfoWindow({
            content: `
              <div class="info-window">
                <p><strong>台风等级:</strong> ${point.level.toFixed(1)}</p>
                <p><strong>${trackType === 'history' ? '历史' : trackType === 'prediction' ? '预测' : '实际'}步骤:</strong> ${point.step}</p>
                <p><strong>位置:</strong> [${point.longitude.toFixed(2)}, ${point.latitude.toFixed(2)}]</p>
                <p><strong>风速:</strong> ${point.speed.toFixed(1)} m/s</p>
                <p><strong>气压:</strong> ${point.pressure.toFixed(1)} hPa</p>
                ${point.rainfall ? `<p><strong>降水量:</strong> ${point.rainfall.toFixed(3)}</p>` : ''}
              </div>
            `,
            anchor: 'bottom-center',
            offset: new AMap.Pixel(0, -10)
          });
          
          infoWindow.open(map.value, [point.longitude, point.latitude]);
        })
        
        markersArray.value.push(marker)
      })
      
      // 添加标记点到地图
      if (markersArray.value.length > 0) {
        map.value.add(markersArray.value)
      }
    }

    // 调整地图视野，确保路径完全可见
    const adjustMapView = () => {
      if (!currentTyphoonData.value || !map.value) return
      
      const { paths } = currentTyphoonData.value
      
      // 收集所有点的经纬度
      const allPoints = []
      
      // 添加历史路径点
      if (paths.history && paths.history.length > 0) {
        paths.history.forEach(point => {
          allPoints.push([point.longitude, point.latitude])
        })
      }
      
      // 添加预测路径点
      if (paths.prediction && paths.prediction.length > 0) {
        paths.prediction.forEach(point => {
          allPoints.push([point.longitude, point.latitude])
        })
      }
      
      // 添加实际路径点
      if (paths.actual && paths.actual.length > 0) {
        paths.actual.forEach(point => {
          allPoints.push([point.longitude, point.latitude])
        })
      }
      
      if (allPoints.length === 0) return
      
      try {
        // 创建边界对象
        const bounds = new AMap.Bounds(
          [Math.min(...allPoints.map(point => point[0])), Math.min(...allPoints.map(point => point[1]))],
          [Math.max(...allPoints.map(point => point[0])), Math.max(...allPoints.map(point => point[1]))]
        )
        
        // 添加一些边距
        const padding = [50, 50, 50, 50]
        
        // 设置地图视野
        map.value.setBounds(bounds, true, padding)
      } catch (error) {
        console.error('调整地图视野出错:', error)
        // 回退到默认视图
        map.value.setZoom(4)
        map.value.setCenter([135.0, 12.0])
      }
    }

    // 调试功能: 切换调试面板显示
    const toggleDebugPanel = () => {
      showDebugPanel.value = !showDebugPanel.value;
    }

    // 调试功能: 获取台风数据统计
    const fetchTyphoonCount = async () => {
      debugLoading.value = true;
      debugInfo.value = null;
      
      try {
        const response = await fetch('http://127.0.0.1:8010/api/debug/typhoon/count');
        const data = await response.json();
        debugInfo.value = data;
        
        if (data.total_records === 0) {
          alert('数据库中没有台风数据！请检查数据导入是否成功。');
        }
      } catch (error) {
        console.error('获取台风数据统计失败:', error);
        alert('获取台风数据统计失败');
      } finally {
        debugLoading.value = false;
      }
    }

    // 调试功能: 获取数据文件路径信息
    const fetchDataPaths = async () => {
      debugLoading.value = true;
      debugPaths.value = null;
      
      try {
        const response = await fetch('http://127.0.0.1:8010/api/debug/typhoon/data_paths');
        const data = await response.json();
        debugPaths.value = data;
      } catch (error) {
        console.error('获取数据文件路径失败:', error);
        alert('获取数据文件路径失败');
      } finally {
        debugLoading.value = false;
      }
    }

    // 调试功能: 获取样本调试数据
    const fetchSampleDebugData = async () => {
      if (!selectedSample.value) return;
      
      debugLoading.value = true;
      sampleDebugData.value = null;
      
      try {
        const response = await fetch(`http://127.0.0.1:8010/api/debug/typhoon/sample_data/${selectedSample.value}`);
        const data = await response.json();
        sampleDebugData.value = data;
      } catch (error) {
        console.error('获取样本调试数据失败:', error);
        alert('获取样本调试数据失败');
      } finally {
        debugLoading.value = false;
      }
    }

    // 调试功能: 重新加载台风数据
    const reloadTyphoonData = async () => {
      if (!confirm('确定要重新加载台风数据吗？这将清除现有数据并重新导入。')) {
        return;
      }
      
      debugLoading.value = true;
      
      try {
        const response = await fetch('http://127.0.0.1:8010/api/debug/typhoon/reload_data', {
          method: 'POST'
        });
        const data = await response.json();
        
        alert(`台风数据重新加载完成！共导入 ${data.total_records} 条记录，${data.distinct_samples} 个样本。`);
        
        // 重新获取样本列表
        await fetchTyphoonSamples();
      } catch (error) {
        console.error('重新加载台风数据失败:', error);
        alert('重新加载台风数据失败');
      } finally {
        debugLoading.value = false;
      }
    }

    onMounted(() => {
      // 检查是否已登录
      checkAuth()
      
      // 如果已登录，初始化地图
      if (currentUser.value) {
        nextTick(() => {
          initMap()
          fetchTyphoonSamples()
        })
      }
    })

    return {
      currentView,
      currentUser,
      apiStatus,
      typhoonSamples,
      selectedSample,
      currentTyphoonData,
      isLoading,
      handleLoginSuccess,
      handleRegisterSuccess,
      logout,
      handleSampleChange,
      showTyphoonPath,
      // 调试相关
      showDebugPanel,
      debugLoading,
      debugInfo,
      debugPaths,
      sampleDebugData,
      viewSampleDetails,
      toggleDebugPanel,
      fetchTyphoonCount,
      fetchDataPaths,
      fetchSampleDebugData,
      reloadTyphoonData
    }
  }
}
</script>

<style scoped>
a {
  color: #42b983;
}

.layout-container {
  position: relative;
  width: 100%;
  height: 100vh;
  display: flex;
  flex-direction: column;
}

.map-container {
  width: 100%;
  height: calc(100vh - 130px); /* 减去顶部区域的高度 */
  margin: 0;
  border: 1px solid #ddd;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
}

.auth-section {
  max-width: 450px;
  margin: 0 auto 30px;
}

.user-profile {
  position: absolute;
  top: 10px;
  right: 10px;
  z-index: 100;
  padding: 10px 15px;
  border: 1px solid #ddd;
  border-radius: 8px;
  background-color: #f9f9f9;
  text-align: center;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  display: flex;
  align-items: center;
  gap: 10px;
}

.user-profile p {
  margin: 0;
  font-size: 0.9rem;
  font-weight: bold;
}

.logout-btn {
  background-color: #ff4c4c;
  color: white;
  padding: 4px 8px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 0.8rem;
  margin: 0;
}

.logout-btn:hover {
  background-color: #e04141;
}

.typhoon-selector {
  padding: 10px;
  background-color: #f5f5f5;
  border-radius: 8px 8px 0 0;
  box-shadow: 0 1px 4px rgba(0,0,0,0.1);
  display: flex;
  flex-direction: row;
  justify-content: space-between;
  align-items: center;
  height: 60px; /* 固定高度 */
}
.typhoon-selector {
  padding: 10px;
  background-color: #f5f5f5;
  border-radius: 8px 8px 0 0;
  box-shadow: 0 1px 4px rgba(0,0,0,0.1);
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: center;
  height: 60px; /* 固定高度 */
}

.typhoon-info {
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 0 20px;
}

.legend {
  display: flex;
  gap: 20px;
  justify-content: center;
}
.selector-container {
  display: flex;
  gap: 10px;
  min-width: 300px;
}

.selector-container select {
  flex-grow: 1;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
  font-size: 14px;
}

.show-path-btn {
  background-color: #42b983;
  color: white;
  padding: 8px 16px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
}

.show-path-btn:disabled {
  background-color: #a8d5c2;
  cursor: not-allowed;
}

.typhoon-info {
  display: flex;
  align-items: center;
  padding-left: 20px;
}

.legend {
  display: flex;
  gap: 20px;
}

.legend-item {
  display: flex;
  align-items: center;
  gap: 5px;
}

.path-color {
  width: 20px;
  height: 4px;
  border-radius: 2px;
}

.path-color.history {
  background-color: #32CD32; /* 历史路径为绿色 */
}

.path-color.prediction {
  background-color: #1874CD; /* 预测路径为蓝色 */
}

.path-color.actual {
  background-color: #FF3030; /* 实际路径为红色 */
}

.data-info {
  font-size: 0.9rem;
  color: #666;
  margin: 5px 0;
}

/* 台风标记点样式 */
:deep(.typhoon-marker) {
  width: 20px;
  height: 20px;
  border-radius: 50%;
  text-align: center;
  line-height: 20px;
  font-size: 12px;
  font-weight: bold;
  color: white;
}

:deep(.prediction-marker) {
  background-color: #1874CD;
}

:deep(.actual-marker) {
  background-color: #FF3030;
}

:deep(.history-marker) {
  background-color: #32CD32;
}

:deep(.info-window) {
  padding: 5px;
  min-width: 200px;
}

button {
  margin: 0;
  padding: 8px 16px;
  background-color: #42b983;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

button:hover {
  background-color: #3aa876;
}
/* 调试面板样式 */
.debug-panel {
  position: absolute; 
  top: 1px;
  left: 10px;
  z-index: 100;
  width: auto;
  height: auto;
  max-width: 300px;
  margin-top: 0;
  padding: 10px;
  background-color: rgba(248, 248, 248, 0.9);
  border: 1px solid #ddd;
  border-radius: 6px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.15);
}

/* 为高德地图的标记点标签设置更小的字体 */
:deep(.amap-marker-label) {
  font-size: 10px !important;
  padding: 1px 3px !important;
}

.debug-panel h3 {
  margin-top: 0;
  color: #333;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.toggle-btn {
  font-size: 0.8em;
  color: #42b983;
  cursor: pointer;
  font-weight: normal;
}

.debug-actions {
  display: flex;
  gap: 10px;
  margin-bottom: 15px;
  flex-wrap: wrap;
}

.debug-btn {
  padding: 6px 12px;
  background-color: #42b983;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
}

.debug-btn.warning {
  background-color: #e6a23c;
}

.debug-btn.small {
  padding: 3px 8px;
  font-size: 12px;
}

.debug-loading {
  margin: 15px 0;
  font-style: italic;
  color: #666;
}

.debug-info {
  margin-top: 15px;
  border-top: 1px solid #ddd;
  padding-top: 15px;
}

.debug-info h4 {
  margin-top: 0;
  color: #333;
  font-size: 16px;
}

.debug-info pre {
  background-color: #f1f1f1;
  padding: 10px;
  border-radius: 4px;
  overflow: auto;
  max-height: 300px;
  font-size: 13px;
}

.sample-data {
  margin-top: 10px;
}

.sample-data p {
  margin: 5px 0;
}

@media (min-aspect-ratio: 16/9) {
  /* 当屏幕宽高比例大于16:9时应用 */
  .map-container {
    height: calc(100vh - 100px);
  }
  
  .typhoon-selector {
    height: 50px;
  }
}
/* 调试面板样式 */
.debug-panel {
  position: absolute; 
  top: 1px;
  left: 10px;
  z-index: 100;
  width: auto;
  height: auto;
  max-width: 300px;
  margin-top: 0;
  padding: 10px;
  background-color: rgba(248, 248, 248, 0.9);
  border: 1px solid #ddd;
  border-radius: 6px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.15);
}

.debug-panel h3 {
  margin-top: 0;
  color: #333;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.toggle-btn {
  font-size: 0.8em;
  color: #42b983;
  cursor: pointer;
  font-weight: normal;
}

.debug-actions {
  display: flex;
  gap: 10px;
  margin-bottom: 15px;
  flex-wrap: wrap;
}

.debug-btn {
  padding: 6px 12px;
  background-color: #42b983;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
}

.debug-btn.warning {
  background-color: #e6a23c;
}

.debug-btn.small {
  padding: 3px 8px;
  font-size: 12px;
}

.debug-loading {
  margin: 15px 0;
  font-style: italic;
  color: #666;
}

.debug-info {
  margin-top: 15px;
  border-top: 1px solid #ddd;
  padding-top: 15px;
}

.debug-info h4 {
  margin-top: 0;
  color: #333;
  font-size: 16px;
}

.debug-info pre {
  background-color: #f1f1f1;
  padding: 10px;
  border-radius: 4px;
  overflow: auto;
  max-height: 300px;
  font-size: 13px;
}

.sample-data {
  margin-top: 10px;
}

.sample-data p {
  margin: 5px 0;
}

@media (min-aspect-ratio: 16/9) {
  /* 当屏幕宽高比例大于16:9时应用 */
  .map-container {
    height: calc(100vh - 100px);
  }
  
  .typhoon-selector {
    height: 50px;
  }
}
</style>
