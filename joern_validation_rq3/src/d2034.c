void InputMethodIBus::CommitText(const std::string& text) {
  if (suppress_next_result_ || text.empty())
    return;

  if (!GetTextInputClient())
    return;

  const string16 utf16_text = UTF8ToUTF16(text);
  if (utf16_text.empty())
    return;

  result_text_.append(utf16_text);

  if (pending_key_events_.empty() && !IsTextInputTypeNone()) {
    GetTextInputClient()->InsertText(utf16_text);
    result_text_.clear();
  }
}
