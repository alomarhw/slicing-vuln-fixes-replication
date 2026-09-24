void RenderWidgetHostImpl::DidDeleteSharedBitmap(
    const viz::SharedBitmapId& id) {
  shared_bitmap_manager_->ChildDeletedSharedBitmap(id);
  owned_bitmaps_.erase(id);
}
