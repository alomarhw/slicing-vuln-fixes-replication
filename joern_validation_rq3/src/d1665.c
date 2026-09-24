void AutofillManager::FillFieldWithValue(AutofillField* autofill_field,
                                         const AutofillDataModel& data_model,
                                         FormFieldData* field_data,
                                         bool should_notify,
                                         const base::string16& cvc) {
  if (field_filler_.FillFormField(*autofill_field, data_model, field_data,
                                  cvc)) {
    autofill_field->is_autofilled = true;

    field_data->is_autofilled = true;
    AutofillMetrics::LogUserHappinessMetric(
        AutofillMetrics::FIELD_WAS_AUTOFILLED, autofill_field->Type().group(),
        client_->GetSecurityLevelForUmaHistograms());

    if (should_notify) {
      client_->DidFillOrPreviewField(
          /*value=*/data_model.GetInfo(autofill_field->Type(), app_locale_),
          /*profile_full_name=*/data_model.GetInfo(AutofillType(NAME_FULL),
                                                   app_locale_));
    }
  }
}
