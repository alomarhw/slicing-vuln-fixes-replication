ScopedSimulateModifierKeyPress::ScopedSimulateModifierKeyPress(
    WebContents* web_contents,
    bool control,
    bool shift,
    bool alt,
    bool command)
    : web_contents_(web_contents),
      modifiers_(0),
      control_(control),
      shift_(shift),
      alt_(alt),
      command_(command) {
  modifiers_ =
      SimulateModifierKeysDown(web_contents_, control_, shift_, alt_, command_);
}
