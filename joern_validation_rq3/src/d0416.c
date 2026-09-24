void PasswordAutofillAgent::DidCommitProvisionalLoad(
    bool is_same_document_navigation,
    ui::PageTransition transition) {
  if (!is_same_document_navigation) {
    checked_safe_browsing_reputation_ = false;
    recorded_first_filling_result_ = false;
  }
}
