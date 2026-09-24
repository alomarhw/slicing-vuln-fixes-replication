int64_t MetricsLog::GetCurrentTime() {
  return (base::TimeTicks::Now() - base::TimeTicks()).InSeconds();
}
