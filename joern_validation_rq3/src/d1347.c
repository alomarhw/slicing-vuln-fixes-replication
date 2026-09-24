void SynchronizeVisualPropertiesMessageFilter::OnUpdatedSurfaceIdOnUI(
    viz::LocalSurfaceId surface_id) {
  last_surface_id_ = surface_id;
  if (surface_id_run_loop_) {
    surface_id_run_loop_->QuitWhenIdle();
  }
}
