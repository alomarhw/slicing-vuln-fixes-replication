void ChromePasswordManagerClient::HideManualFallbackForSaving() {
  if (!CanShowBubbleOnURL(web_contents()->GetLastCommittedURL()))
    return;

#if !defined(OS_ANDROID)
  PasswordsClientUIDelegate* manage_passwords_ui_controller =
      PasswordsClientUIDelegateFromWebContents(web_contents());
  if (manage_passwords_ui_controller)
    manage_passwords_ui_controller->OnHideManualFallbackForSaving();
#endif  // !defined(OS_ANDROID)
}
