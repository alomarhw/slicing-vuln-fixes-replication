void RenderViewImpl::didChangeSelection(bool is_empty_selection) {
  if (!handling_input_event_ && !handling_select_range_)
    return;

  if (is_empty_selection)
    selection_text_.clear();

  SyncSelectionIfRequired();
  UpdateTextInputState(DO_NOT_SHOW_IME);
}
