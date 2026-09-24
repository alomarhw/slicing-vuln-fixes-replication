void GpuCommandBufferStub::OnSetSurfaceVisible(bool visible) {
  DCHECK(surface_state_.get());
  surface_state_->visible = visible;
  surface_state_->last_used_time = base::TimeTicks::Now();
  channel_->gpu_channel_manager()->gpu_memory_manager()->ScheduleManage();
}
