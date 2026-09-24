void TabStrip::DraggedTabsDetached() {
  controller()->OnStoppedDraggingTabs();
  newtab_button_->SetVisible(true);
}
