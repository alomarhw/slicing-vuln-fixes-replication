bool Textfield::Cut() {
  if (!read_only() && text_input_type_ != ui::TEXT_INPUT_TYPE_PASSWORD &&
      model_->Cut()) {
    if (controller_)
      controller_->OnAfterCutOrCopy(ui::CLIPBOARD_TYPE_COPY_PASTE);
    return true;
  }
  return false;
}
