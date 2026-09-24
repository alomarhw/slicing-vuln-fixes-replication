void PaymentRequestState::OnSpecUpdated() {
  autofill::AutofillProfile* selected_shipping_profile =
      selected_shipping_profile_;
  autofill::AutofillProfile* selected_contact_profile =
      selected_contact_profile_;

  if (spec_->current_update_reason() ==
      PaymentRequestSpec::UpdateReason::RETRY) {
    if (spec_->has_shipping_address_error() && selected_shipping_profile) {
      invalid_shipping_profile_ = selected_shipping_profile;
      selected_shipping_profile_ = nullptr;
    }

    if (spec_->has_payer_error() && selected_contact_profile) {
      invalid_contact_profile_ = selected_contact_profile;
      selected_contact_profile_ = nullptr;
    }
  }

  if (spec_->selected_shipping_option_error().empty()) {
    selected_shipping_option_error_profile_ = nullptr;
  } else {
    selected_shipping_option_error_profile_ = selected_shipping_profile;
    selected_shipping_profile_ = nullptr;
    if (spec_->has_shipping_address_error() && selected_shipping_profile) {
      invalid_shipping_profile_ = selected_shipping_profile;
    }
  }

  is_waiting_for_merchant_validation_ = false;
  UpdateIsReadyToPayAndNotifyObservers();
}
