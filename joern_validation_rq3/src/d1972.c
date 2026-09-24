void GpuProcessHost::OnInitialized(bool result) {
  UMA_HISTOGRAM_BOOLEAN("GPU.GPUProcessInitialized", result);
}
