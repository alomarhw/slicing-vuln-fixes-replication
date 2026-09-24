void Instance::DestroyHorizontalScrollbar() {
  if (!h_scrollbar_.get())
    return;
  if (h_scrollbar_->GetValue())
    engine_->ScrolledToXPosition(0);
  h_scrollbar_.reset();
}
