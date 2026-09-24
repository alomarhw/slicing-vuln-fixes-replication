void OmniboxViewViews::AddedToWidget() {
  views::Textfield::AddedToWidget();
  scoped_observer_.Add(GetWidget()->GetCompositor());
}
