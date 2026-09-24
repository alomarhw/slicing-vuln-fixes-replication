void Label::SetTooltipText(const std::wstring& tooltip_text) {
  tooltip_text_ = WideToUTF16Hack(tooltip_text);
}
