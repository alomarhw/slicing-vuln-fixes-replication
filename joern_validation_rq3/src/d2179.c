void AutoFillManager::DeterminePossibleFieldTypesForUpload(
    FormStructure* submitted_form) {
  for (size_t i = 0; i < submitted_form->field_count(); i++) {
    const AutoFillField* field = submitted_form->field(i);
    FieldTypeSet field_types;
    personal_data_->GetPossibleFieldTypes(field->value(), &field_types);
    DCHECK(!field_types.empty());
    submitted_form->set_possible_types(i, field_types);
  }
}
