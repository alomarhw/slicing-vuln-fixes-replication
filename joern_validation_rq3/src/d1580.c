bool AutofillManager::ShouldShowScanCreditCard(const FormData& form,
                                               const FormFieldData& field) {
  if (!client_->HasCreditCardScanFeature())
    return false;

  AutofillField* autofill_field = GetAutofillField(form, field);
  if (!autofill_field)
    return false;

  bool is_card_number_field =
      autofill_field->Type().GetStorableType() == CREDIT_CARD_NUMBER &&
      base::ContainsOnlyChars(CreditCard::StripSeparators(field.value),
                              base::ASCIIToUTF16("0123456789"));

  bool is_scannable_name_on_card_field =
      autofill_field->Type().GetStorableType() == CREDIT_CARD_NAME_FULL &&
      base::FeatureList::IsEnabled(kAutofillScanCardholderName);

  if (!is_card_number_field && !is_scannable_name_on_card_field)
    return false;

  if (IsFormNonSecure(form))
    return false;

  static const int kShowScanCreditCardMaxValueLength = 6;
  return field.value.size() <= kShowScanCreditCardMaxValueLength;
}
