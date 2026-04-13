<template>
  <el-card 
    class="sensor-card glass-panel state-transition hover:scale-[1.02]"
    :body-style="{ padding: '20px' }"
  >
    <div class="flex items-center gap-4">
      <div 
        :class="[
          'p-3 rounded-xl transition-all duration-500',
          active ? 'bg-primary/10 text-primary animate-glow' : 'bg-slate-100 text-slate-400'
        ]"
      >
        <el-icon :size="24">
          <component :is="icon" />
        </el-icon>
      </div>
      
      <div class="flex-1">
        <p class="text-[10px] font-black text-slate-400 uppercase tracking-widest mb-0.5">{{ label }}</p>
        <div class="flex items-baseline gap-1">
          <span class="text-2xl font-black text-slate-900 tracking-tight tabular-nums transition-all duration-300">
            {{ displayValue }}
          </span>
          <span class="text-xs font-bold text-slate-400">{{ unit }}</span>
        </div>
      </div>

      <!-- Trend indicator (Mock) -->
      <div class="flex flex-col items-center gap-1">
        <div 
          :class="[
            'text-[10px] font-bold px-1.5 py-0.5 rounded-full',
            trend === 'up' ? 'text-green-400 bg-green-400/10' : 'text-blue-400 bg-blue-400/10'
          ]"
        >
          {{ trend === 'up' ? '↑' : '↓' }}
        </div>
      </div>
    </div>
  </el-card>
</template>

<script setup>
import { ref, watch, onMounted } from 'vue'

const props = defineProps({
  label: String,
  value: [Number, String],
  unit: String,
  icon: Object,
  active: {
    type: Boolean,
    default: true
  }
})

const displayValue = ref(0)
const trend = ref('up')

// Smooth value transition logic
watch(() => props.value, (newVal, oldVal) => {
  const start = parseFloat(displayValue.value) || 0
  const end = parseFloat(newVal) || 0
  
  trend.value = end >= start ? 'up' : 'down'
  
  // Quick animation to the new value
  let startTime = null
  const duration = 500 // ms
  
  const animate = (timestamp) => {
    if (!startTime) startTime = timestamp
    const progress = Math.min((timestamp - startTime) / duration, 1)
    const current = start + (end - start) * progress
    
    displayValue.value = current.toFixed(1)
    
    if (progress < 1) {
      requestAnimationFrame(animate)
    } else {
      displayValue.value = end.toFixed(1)
    }
  }
  
  requestAnimationFrame(animate)
}, { immediate: true })

onMounted(() => {
  displayValue.value = parseFloat(props.value) || 0
})
</script>

<style scoped>
.sensor-card {
  border-radius: 1.25rem;
  overflow: hidden;
}

:deep(.el-card__body) {
  position: relative;
  z-index: 1;
}

.sensor-card::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: radial-gradient(circle at top right, rgba(255, 255, 255, 0.05), transparent 60%);
  pointer-events: none;
}
</style>
