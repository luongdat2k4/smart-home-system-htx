<template>
  <div class="glass-card p-6 flex items-center gap-5 transition-all duration-300 hover:bg-slate-50">
    <div 
      class="p-3.5 rounded-2xl bg-slate-100 text-slate-400 group-hover:text-slate-600 transition-colors"
      :class="[active ? colorClass : 'text-slate-300']"
    >
      <component :is="icon" :size="24" />
    </div>
    
    <div class="flex-1">
      <p class="text-[11px] font-black text-slate-400 uppercase tracking-widest mb-1">{{ label }}</p>
      <div class="flex items-baseline gap-1.5">
        <span class="text-3xl font-black text-slate-800 tabular-nums">
          {{ displayValue }}
        </span>
        <span class="text-sm font-bold text-slate-500">{{ unit }}</span>
      </div>
    </div>

    <!-- Trend Indicator -->
    <div class="flex flex-col items-center">
      <div 
        class="text-[10px] font-black px-2 py-1 rounded-lg flex items-center gap-1"
        :class="[trend === 'up' ? 'text-cyan-400 bg-cyan-400/10' : 'text-purple-400 bg-purple-400/10']"
      >
        <TrendingUp v-if="trend === 'up'" :size="12" />
        <TrendingDown v-else :size="12" />
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, onMounted } from 'vue'
import { TrendingUp, TrendingDown } from 'lucide-vue-next'

const props = defineProps({
  label: String,
  value: [Number, String],
  unit: String,
  icon: [Object, Function],
  active: {
    type: Boolean,
    default: true
  },
  colorClass: {
    type: String,
    default: 'text-primary'
  }
})

const displayValue = ref(0)
const trend = ref('up')

watch(() => props.value, (newVal, oldVal) => {
  const start = parseFloat(displayValue.value) || 0
  const end = parseFloat(newVal) || 0
  
  trend.value = end >= start ? 'up' : 'down'
  
  // Quick animation to the new value
  const duration = 500
  const startTime = performance.now()
  
  const animate = (time) => {
    const elapsed = time - startTime
    const progress = Math.min(elapsed / duration, 1)
    
    // Ease out expo
    const ease = progress === 1 ? 1 : 1 - Math.pow(2, -10 * progress)
    const current = start + (end - start) * ease
    
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
