gfx::Size ClientControlledShellSurface::GetMaximumSize() const {
  if (can_maximize_) {
    return gfx::Size();
  } else {
    return ShellSurfaceBase::GetMaximumSize();
  }
}
