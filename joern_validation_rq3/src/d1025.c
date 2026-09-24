InputMethodDescriptor CreateInputMethodDescriptor(
    const std::string& id,
    const std::string& display_name,
    const std::string& raw_layout,
    const std::string& language_code) {
  static const char fallback_layout[] = "us";
  std::string physical_keyboard_layout = fallback_layout;
  const std::string& virtual_keyboard_layout = raw_layout;

  std::vector<std::string> layout_names;
  base::SplitString(raw_layout, ',', &layout_names);

  for (size_t i = 0; i < layout_names.size(); ++i) {
    if (XkbLayoutIsSupported(layout_names[i])) {
      physical_keyboard_layout = layout_names[i];
      break;
    }
  }

  return InputMethodDescriptor(id,
                               display_name,
                               physical_keyboard_layout,
                               virtual_keyboard_layout,
                                language_code);
 }
