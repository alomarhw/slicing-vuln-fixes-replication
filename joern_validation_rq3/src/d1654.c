void LauncherView::LayoutToIdealBounds() {
  IdealBounds ideal_bounds;
  CalculateIdealBounds(&ideal_bounds);

  if (bounds_animator_->IsAnimating())
    AnimateToIdealBounds();
  else
    views::ViewModelUtils::SetViewBoundsToIdealBounds(*view_model_);

  overflow_button_->SetBoundsRect(ideal_bounds.overflow_bounds);
}
