void Label::SetMultiLine(bool multi_line) {
  DCHECK(!multi_line || !elide_in_middle_);
  if (multi_line != is_multi_line_) {
    is_multi_line_ = multi_line;
    text_size_valid_ = false;
    PreferredSizeChanged();
    SchedulePaint();
  }
}
