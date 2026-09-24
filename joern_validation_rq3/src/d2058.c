MediaControlMuteButtonElement::getOverflowStringName() {
  if (mediaElement().muted())
    return WebLocalizedString::OverflowMenuUnmute;
  return WebLocalizedString::OverflowMenuMute;
}
