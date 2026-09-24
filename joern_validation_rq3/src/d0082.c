void PasswordAutofillAgent::DidEndTextFieldEditing() {
  focus_state_notifier_.FocusedInputChanged(FocusedFieldType::kUnknown);
}
