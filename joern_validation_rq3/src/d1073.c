void LinkInfoBar::Layout() {
  InfoBar::Layout();

  gfx::Size icon_ps = icon_->GetPreferredSize();
  icon_->SetBounds(kHorizontalPadding, OffsetY(this, icon_ps), icon_ps.width(),
                   icon_ps.height());

  int label_1_x = icon_->bounds().right() + kIconLabelSpacing;

  int available_width = GetAvailableWidth() - label_1_x;

  gfx::Size label_1_ps = label_1_->GetPreferredSize();
  label_1_->SetBounds(label_1_x, OffsetY(this, label_1_ps), label_1_ps.width(),
                      label_1_ps.height());

  gfx::Size link_ps = link_->GetPreferredSize();
  bool has_second_label = !label_2_->GetText().empty();
  if (has_second_label) {
    link_->SetBounds(label_1_->bounds().right(),
                     OffsetY(this, link_ps), link_ps.width(), link_ps.height());
  } else {
    link_->SetBounds(label_1_x + available_width - link_ps.width(),
                     OffsetY(this, link_ps), link_ps.width(), link_ps.height());
  }

  gfx::Size label_2_ps = label_2_->GetPreferredSize();
  label_2_->SetBounds(link_->bounds().right(),
                      OffsetY(this, label_2_ps), label_2_ps.width(),
                      label_2_ps.height());
}
