void AutofillPopupItemView::OnMouseReleased(const ui::MouseEvent& event) {
  AutofillPopupController* controller = popup_view_->controller();
  if (controller && event.IsOnlyLeftMouseButton() &&
      HitTestPoint(event.location())) {
    controller->AcceptSuggestion(line_number_);
  }
}
