<template>
  <div 
    class="glass-card p-6 transition-all duration-300 group hover:scale-[1.03] hover:shadow-cyan-500/10 cursor-pointer"
    :class="[modelValue ? 'border-primary/50 bg-primary/5 shadow-primary/20' : 'border-white/40 shadow-slate-200/50']"
    @click="$emit('update:modelValue', !modelValue)"
  >
    <div class="flex items-start justify-between mb-8">
      <div 
        class="p-4 rounded-2xl transition-all duration-500"
        :class="[
          modelValue 
            ? 'bg-primary text-white shadow-lg shadow-primary/40' 
            : 'bg-slate-100 text-slate-400 group-hover:bg-slate-200 group-hover:text-slate-500'
        ]"
      >
        <component 
          :is="icon" 
          :size="28" 
          :class="{ 'animate-spin-slow': modelValue && title.toLowerCase().includes('quạt') }"
        />
      </div>

      <!-- Custom Switch -->
      <div 
        class="w-12 h-6 rounded-full relative transition-colors duration-300"
        :class="modelValue ? 'bg-primary' : 'bg-slate-200'"
      >
        <div 
          class="absolute top-1 left-1 w-4 h-4 bg-white rounded-full transition-transform duration-300 shadow-sm"
          :class="{ 'translate-x-6': modelValue }"
        ></div>
      </div>
    </div>

    <div>
      <h3 class="text-lg font-bold text-slate-800 mb-1">{{ title }}</h3>
      <div class="flex items-center gap-2">
        <span 
          class="w-1.5 h-1.5 rounded-full"
          :class="modelValue ? 'bg-primary animate-pulse' : 'bg-slate-500'"
        ></span>
        <span 
          class="text-sm font-medium transition-colors"
          :class="modelValue ? 'text-primary' : 'text-slate-500'"
        >
          {{ modelValue ? onText : offText }}
        </span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
  title: String,
  icon: [Object, Function],
  modelValue: Boolean,
  onText: {
    type: String,
    default: 'Active'
  },
  offText: {
    type: String,
    default: 'Inactive'
  }
})

defineEmits(['update:modelValue'])
</script>

<style scoped>
.animate-spin-slow {
  animation: spin 3s linear infinite;
}

@keyframes spin {
  from { transform: rotate(0deg); }
  to { transform: rotate(360deg); }
}
</style>
