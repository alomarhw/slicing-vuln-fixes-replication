scoped_ptr<ResizeLock> RenderWidgetHostViewAura::CreateResizeLock(
    bool defer_compositor_lock) {
  gfx::Size desired_size = window_->bounds().size();
  return scoped_ptr<ResizeLock>(new CompositorResizeLock(
      window_->GetDispatcher(),
      desired_size,
      defer_compositor_lock,
      base::TimeDelta::FromMilliseconds(kResizeLockTimeoutMs)));
}
