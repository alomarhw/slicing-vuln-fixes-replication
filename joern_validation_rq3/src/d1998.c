AutofillDialogViews::OverlayView::OverlayView(
    AutofillDialogViewDelegate* delegate)
    : delegate_(delegate),
      image_view_(new views::ImageView()),
      message_view_(new views::Label()) {
  message_view_->SetAutoColorReadabilityEnabled(false);
  message_view_->SetMultiLine(true);

  AddChildView(image_view_);
  AddChildView(message_view_);
}
