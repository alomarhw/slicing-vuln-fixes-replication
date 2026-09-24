DragState& DragController::GetDragState() {
  if (!drag_state_)
    drag_state_ = new DragState;
  return *drag_state_;
}
