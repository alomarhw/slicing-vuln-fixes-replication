void DelegatedFrameHost::LockResources() {
  DCHECK(local_surface_id_.is_valid());
  frame_evictor_->LockFrame();
}
