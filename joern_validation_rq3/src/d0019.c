void LockContentsView::AddedToWidget() {
  DoLayout();

  if (primary_big_view_)
    primary_big_view_->RequestFocus();
}
