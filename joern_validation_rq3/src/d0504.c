void PDFiumEngine::SetEditMode(bool edit_mode) {
  if (!kIsEditModeTracked || edit_mode_ == edit_mode)
    return;

  edit_mode_ = edit_mode;
  client_->IsEditModeChanged(edit_mode_);
}
