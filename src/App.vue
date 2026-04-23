<template>
  <div class="min-h-screen p-4 md:p-8 lg:p-12 max-w-7xl mx-auto">
    <!-- Header Section -->
    <header class="mb-12 flex flex-col md:flex-row md:items-end justify-between gap-6 animate-fade-in">
      <div>
        <div class="flex items-center gap-3 mb-3">
          <div class="p-3 bg-primary text-white rounded-2xl glow-cyan animate-float">
            <LayoutDashboard :size="28" />
          </div>
          <h1 class="text-4xl font-black text-slate-900 tracking-tight">Điều khiển Nhà Thông Minh</h1>
        </div>
        <p class="text-slate-600 font-medium ml-1">Chào mừng trở lại! Hệ thống của bạn đang hoạt động ổn định.</p>
      </div>

      <!-- Connection Status Badge -->
      <div 
        class="flex items-center gap-3 px-5 py-2.5 rounded-2xl glass border-slate-200/50 shadow-sm transition-all duration-500"
        :class="statusStyles.container"
      >
        <div class="relative flex h-3 w-3">
          <span 
            class="animate-ping absolute inline-flex h-full w-full rounded-full opacity-75"
            :class="statusStyles.dot"
          ></span>
          <span class="relative inline-flex rounded-full h-3 w-3" :class="statusStyles.dot"></span>
        </div>
        <div class="flex flex-col">
          <span class="text-[10px] font-black uppercase tracking-widest text-slate-400">Trạng thái hệ thống</span>
          <span class="text-sm font-bold tracking-wide" :class="statusStyles.text">{{ connectionStatusText }}</span>
        </div>
      </div>
    </header>

    <!-- Main Content Section -->
    <main class="space-y-8">
      <!-- Sensor Stats Row -->
      <div class="grid grid-cols-1 md:grid-cols-3 gap-6">
        <sensor-card
          label="Nhiệt độ"
          :value="sensorData.temperature"
          unit="°C"
          :icon="Thermometer"
          color-class="text-cyan-400"
          class="animate-slide-up-1"
        />
        <sensor-card
          label="Độ ẩm"
          :value="sensorData.humidity"
          unit="%"
          :icon="Droplets"
          color-class="text-purple-400"
          class="animate-slide-up-2"
        />
        <sensor-card
          label="Ánh sáng"
          :value="sensorData.light"
          unit="lx"
          :icon="Sun"
          color-class="text-yellow-400"
          class="animate-slide-up-3"
        />
      </div>

      <div class="grid grid-cols-1 lg:grid-cols-3 gap-8">
        <!-- Control Sidebar -->
        <div class="lg:col-span-1 space-y-6">
          <div class="flex items-center gap-2 mb-2 px-2">
            <Zap :size="18" class="text-primary" />
            <h2 class="text-sm font-black text-slate-400 uppercase tracking-[0.2em]">Điều khiển nhanh</h2>
          </div>
          
          <device-card
            v-model="devices.light.isOn"
            title="Đèn Sân Vườn"
            :icon="Lightbulb"
            on-text="Đang bật"
            off-text="Đã tắt"
            class="animate-slide-up-1"
            :disabled="isAutoMode"
          />

          <device-card
            v-model="devices.irrigation.isOn"
            title="Hệ thống Quạt"
            :icon="Fan"
            on-text="Đang quay"
            off-text="Đã dừng"
            class="animate-slide-up-2"
            :disabled="isAutoMode"
          />

          <!-- Auto Mode Partition -->
          <div class="pt-4 mt-4 border-t border-slate-200/50">
            <div class="flex items-center gap-2 mb-4 px-2">
              <Activity :size="18" class="text-accent-purple" />
              <h2 class="text-sm font-black text-slate-400 uppercase tracking-[0.2em]">Chế độ thông minh</h2>
            </div>
            
            <device-card
              v-model="isAutoMode"
              title="Chế độ tự động"
              :icon="Bot"
              on-text="Đang bật"
              off-text="Đã tắt"
              class="animate-slide-up-3"
            />
          </div>
        </div>

        <!-- Main Analytics Area -->
        <div class="lg:col-span-2">
          <chart-card 
            title="Biểu đồ môi trường" 
            :data="sensorHistory"
            class="animate-slide-up-3"
          />
        </div>
      </div>
    </main>
  </div>
</template>

<script setup>
import { reactive, onMounted, watch, ref, computed } from 'vue'
import mqtt from 'mqtt'
import { 
  LayoutDashboard, 
  Thermometer, 
  Droplets, 
  Sun, 
  Lightbulb, 
  Fan, 
  Zap, 
  Plus,
  Activity,
  AlertCircle,
  Bot
} from 'lucide-vue-next'
import DeviceCard from './components/DeviceCard.vue'
import SensorCard from './components/SensorCard.vue'
import ChartCard from './components/ChartCard.vue'
import { sensorHistory, addDataPoint } from './utils/sensorStore'

// MQTT Configuration
const MQTT_CONFIG = {
  host: '41dbbd06096c41eaa83d9d7dac2297f8.s1.eu.hivemq.cloud',
  port: 8884,
  protocol: 'wss',
  username: 'esp32',
  password: 'VuTran2000@',
  path: '/mqtt'
}

const connectionStatus = ref('Disconnected')
let client = null
let heartbeatInterval = null

// Real-time sensor display
const sensorData = reactive({
  temperature: 0,
  humidity: 0,
  light: 0
})

const isAutoMode = ref(false)

// Status styles mapping
const connectionStatusText = computed(() => {
  switch (connectionStatus.value) {
    case 'Connected': return 'Đã kết nối'
    case 'Connecting...': return 'Đang kết nối...'
    case 'Error': return 'Lỗi kết nối'
    default: return 'Mất kết nối'
  }
})

const statusStyles = computed(() => {
  switch (connectionStatus.value) {
    case 'Connected':
      return { container: 'shadow-primary/10', dot: 'bg-primary', text: 'text-primary' }
    case 'Connecting...':
      return { container: 'shadow-yellow-500/10', dot: 'bg-yellow-500', text: 'text-yellow-500' }
    case 'Error':
      return { container: 'shadow-red-500/10', dot: 'bg-red-500', text: 'text-red-500' }
    default:
      return { container: 'opacity-50', dot: 'bg-slate-500', text: 'text-slate-500' }
  }
})

// Device state management
const devices = reactive({
  light: {
    isOn: false,
    topic: 'home/livingroom/light',
    key: 'led'
  },
  irrigation: {
    isOn: false,
    topic: 'home/garden/irrigation',
    key: 'water'
  }
})

const connectMQTT = () => {
  connectionStatus.value = 'Connecting...'
  const url = `${MQTT_CONFIG.protocol}://${MQTT_CONFIG.host}:${MQTT_CONFIG.port}${MQTT_CONFIG.path}`
  
  client = mqtt.connect(url, {
    username: MQTT_CONFIG.username,
    password: MQTT_CONFIG.password,
    clientId: 'vue_dashboard_' + Math.random().toString(16).substring(2, 8)
  })

  client.on('connect', () => {
    connectionStatus.value = 'Connected'
    const topics = [
      'esp32/nhietdo', 
      'esp32/doam', 
      'esp32/anhsang',
      devices.light.topic,
      devices.irrigation.topic,
      'esp32/auto'
    ]
    client.subscribe(topics, (err) => {
      if (!err) console.log('Subscribed to all topics')
    })
  })

  client.on('message', (topic, message) => {
    try {
      const payloadString = message.toString()
      
      // Handle numeric sensor data
      if (topic.startsWith('esp32/')) {
        const val = parseFloat(payloadString)
        if (!isNaN(val)) {
          if (topic === 'esp32/nhietdo') {
            sensorData.temperature = val.toFixed(1)
            addDataPoint('temperature', val)
            return
          } else if (topic === 'esp32/doam') {
            sensorData.humidity = val.toFixed(1)
            addDataPoint('humidity', val)
            return
          } else if (topic === 'esp32/anhsang') {
            sensorData.light = val.toFixed(1)
            addDataPoint('light', val)
            return
          }
        }
      }

      // Handle JSON device status
      const data = JSON.parse(payloadString)
      
      // Handle Auto Mode Sync
      if (topic === 'esp32/auto' && data.hasOwnProperty('auto')) {
        if (isAutoMode.value !== data.auto) {
          isAutoMode.value = data.auto
        }
        return
      }

      // Sync Light/Fan ONLY if NOT in Auto Mode
      if (!isAutoMode.value) {
        if (topic === devices.light.topic && data.hasOwnProperty(devices.light.key)) {
          if (devices.light.isOn !== data[devices.light.key]) {
            devices.light.isOn = data[devices.light.key]
          }
        } else if (topic === devices.irrigation.topic && data.hasOwnProperty(devices.irrigation.key)) {
          if (devices.irrigation.isOn !== data[devices.irrigation.key]) {
            devices.irrigation.isOn = data[devices.irrigation.key]
          }
        }
      }
    } catch (e) {
      // Not JSON or other error
      console.warn('Received non-standard message on topic:', topic, message.toString())
    }
  })

  client.on('error', (err) => {
    connectionStatus.value = 'Error'
    console.error('MQTT Connection Error:', err)
  })

  client.on('close', () => {
    connectionStatus.value = 'Disconnected'
  })
}

const publishEvent = (topic, payload) => {
  if (client && client.connected) {
    client.publish(topic, JSON.stringify(payload), { qos: 1 })
  }
}

watch(() => devices.light.isOn, (newVal) => {
  publishEvent(devices.light.topic, { [devices.light.key]: newVal })
})

watch(() => devices.irrigation.isOn, (newVal) => {
  publishEvent(devices.irrigation.topic, { [devices.irrigation.key]: newVal })
})

watch(isAutoMode, (newVal) => {
  publishEvent('esp32/auto', { auto: newVal })
  if (newVal) {
    devices.light.isOn = false
    devices.irrigation.isOn = false
  }
})

onMounted(() => {
  connectMQTT()
  
  // Heartbeat every 3 seconds
  heartbeatInterval = setInterval(() => {
    if (client && client.connected) {
      if (isAutoMode.value) {
        // Only heartbeat "auto" when active
        publishEvent('esp32/auto', { auto: true })
      } else {
        // Full heartbeat when auto is inactive
        publishEvent(devices.light.topic, { [devices.light.key]: devices.light.isOn })
        publishEvent(devices.irrigation.topic, { [devices.irrigation.key]: devices.irrigation.isOn })
        publishEvent('esp32/auto', { auto: false })
      }
    }
  }, 3000)
})

import { onUnmounted } from 'vue'
onUnmounted(() => {
  if (heartbeatInterval) clearInterval(heartbeatInterval)
  if (client) client.end()
})
</script>

<style scoped>
@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}

@keyframes slideUp {
  from { opacity: 0; transform: translateY(20px); }
  to { opacity: 1; transform: translateY(0); }
}

.animate-fade-in { animation: fadeIn 1s ease-out; }
.animate-slide-up-1 { animation: slideUp 0.6s ease-out 0.1s both; }
.animate-slide-up-2 { animation: slideUp 0.6s ease-out 0.2s both; }
.animate-slide-up-3 { animation: slideUp 0.6s ease-out 0.3s both; }
</style>