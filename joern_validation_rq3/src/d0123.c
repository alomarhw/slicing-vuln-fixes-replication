  virtual void Layout() {
    int x = kHorizOuterMargin;
    int y = kVertOuterMargin;

    icon_->SetBounds(x, y, kIconSize, kIconSize);
    x += kIconSize;
    x += views::kPanelHorizMargin;

    y += kRightcolumnVerticalShift;
    heading_->SizeToFit(kRightColumnWidth);
    heading_->SetX(x);
    heading_->SetY(y);
    y += heading_->height();
    y += kVertInnerMargin;

    if (info_) {
      info_->SizeToFit(kRightColumnWidth);
      info_->SetX(x);
      info_->SetY(y);
      y += info_->height();
      y += kVertInnerMargin;
    }

    manage_->SizeToFit(kRightColumnWidth);
    manage_->SetX(x);
    manage_->SetY(y);
    y += manage_->height();
    y += kVertInnerMargin;

    gfx::Size sz;
    x += kRightColumnWidth + 2 * views::kPanelHorizMargin + kHorizOuterMargin -
        close_button_->GetPreferredSize().width();
    y = kVertOuterMargin;
    sz = close_button_->GetPreferredSize();
    close_button_->SetBounds(x - 1, y - 1, sz.width(), sz.height());
  }
