bool Browser::ShouldStartShutdown() const {
  if (IsBrowserClosing())
    return false;

  const size_t closing_browsers_count =
      BrowserList::GetInstance()->currently_closing_browsers().size();
  return BrowserList::GetInstance()->size() == closing_browsers_count + 1u;
}
