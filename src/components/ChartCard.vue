<template>
  <div class="glass-card p-6">
    <div class="flex items-center justify-between mb-8">
      <div class="flex items-center gap-4">
        <div class="p-2.5 bg-primary/20 text-primary rounded-xl">
          <TrendingUp :size="24" />
        </div>
        <div>
          <h2 class="text-xl font-bold text-slate-800">{{ title }}</h2>
          <p class="text-xs font-medium text-slate-400 uppercase tracking-widest mt-0.5">Dòng dữ liệu thời gian thực</p>
        </div>
      </div>
      <div class="flex items-center gap-2">
        <span class="w-2 h-2 rounded-full bg-primary animate-pulse"></span>
        <span class="text-[10px] font-black text-slate-400 uppercase tracking-widest">Trực tiếp</span>
      </div>
    </div>
    
    <div ref="chartContainer" class="w-full h-[350px]"></div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, watch } from 'vue'
import * as echarts from 'echarts'
import { TrendingUp } from 'lucide-vue-next'

const props = defineProps({
  title: {
    type: String,
    default: 'Phân tích xu hướng'
  },
  data: {
    type: Object,
    required: true
  }
})

const chartContainer = ref(null)
let chart = null

const initChart = () => {
  if (!chartContainer.value) return
  
  chart = echarts.init(chartContainer.value)
  
  const option = {
    backgroundColor: 'transparent',
    tooltip: {
      trigger: 'axis',
      backgroundColor: 'rgba(255, 255, 255, 0.95)',
      borderColor: 'rgba(0, 0, 0, 0.05)',
      textStyle: { color: '#0f172a' },
      borderWidth: 1,
      borderRadius: 12,
      padding: [10, 15],
      shadowColor: 'rgba(0, 0, 0, 0.1)',
      shadowBlur: 10
    },
    legend: {
      data: ['Nhiệt độ', 'Độ ẩm', 'Ánh sáng'],
      bottom: 0,
      textStyle: { color: '#64748b', fontSize: 12 },
      icon: 'circle'
    },
    grid: {
      top: '10%',
      left: '3%',
      right: '4%',
      bottom: '15%',
      containLabel: true
    },
    xAxis: {
      type: 'category',
      boundaryGap: false,
      data: props.data.timestamps,
      axisLine: { lineStyle: { color: '#e2e8f0' } },
      axisLabel: { color: '#94a3b8' },
      splitLine: { show: false }
    },
    yAxis: [
      {
        type: 'value',
        name: '°C / %',
        min: 0,
        max: 100,
        axisLine: { show: false },
        axisLabel: { color: '#94a3b8' },
        splitLine: { lineStyle: { color: '#f1f5f9' } }
      },
      {
        type: 'value',
        name: 'lx',
        position: 'right',
        axisLine: { show: false },
        axisLabel: { color: '#94a3b8' },
        splitLine: { show: false }
      }
    ],
    series: [
      {
        name: 'Nhiệt độ',
        type: 'line',
        smooth: true,
        showSymbol: false,
        data: props.data.temperature,
        lineStyle: { width: 3, color: '#06b6d4' },
        itemStyle: { color: '#06b6d4' },
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(6, 182, 212, 0.2)' },
            { offset: 1, color: 'rgba(6, 182, 212, 0)' }
          ])
        }
      },
      {
        name: 'Độ ẩm',
        type: 'line',
        smooth: true,
        showSymbol: false,
        data: props.data.humidity,
        lineStyle: { width: 3, color: '#a855f7' },
        itemStyle: { color: '#a855f7' },
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(168, 85, 247, 0.2)' },
            { offset: 1, color: 'rgba(168, 85, 247, 0)' }
          ])
        }
      },
      {
        name: 'Ánh sáng',
        type: 'line',
        yAxisIndex: 1,
        smooth: true,
        showSymbol: false,
        data: props.data.light,
        lineStyle: { width: 2, color: '#10b981', type: 'dashed' },
        itemStyle: { color: '#10b981' }
      }
    ]
  }
  
  chart.setOption(option)
}

watch(() => props.data.timestamps, () => {
  chart?.setOption({
    xAxis: { data: props.data.timestamps },
    series: [
      { data: props.data.temperature },
      { data: props.data.humidity },
      { data: props.data.light }
    ]
  })
}, { deep: true })

onMounted(() => {
  initChart()
  window.addEventListener('resize', () => chart?.resize())
})

onUnmounted(() => {
  window.removeEventListener('resize', () => chart?.resize())
  chart?.dispose()
})
</script>
