bool AutofillDialogViews::ValidateGroup(const DetailsGroup& group,
                                        ValidationType validation_type) {
  DCHECK(group.container->visible());

  FieldValueMap detail_outputs;

  if (group.manual_input->visible()) {
    for (TextfieldMap::const_iterator iter = group.textfields.begin();
         iter != group.textfields.end(); ++iter) {
      if (!iter->second->editable())
        continue;

      detail_outputs[iter->first] = iter->second->GetText();
    }
    for (ComboboxMap::const_iterator iter = group.comboboxes.begin();
         iter != group.comboboxes.end(); ++iter) {
      if (!iter->second->enabled())
        continue;

      views::Combobox* combobox = iter->second;
      base::string16 item =
          combobox->model()->GetItemAt(combobox->selected_index());
      detail_outputs[iter->first] = item;
    }
  } else if (group.section == GetCreditCardSection()) {
    ExpandingTextfield* cvc = group.suggested_info->textfield();
    if (cvc->visible())
      detail_outputs[CREDIT_CARD_VERIFICATION_CODE] = cvc->GetText();
  }

  ValidityMessages validity = delegate_->InputsAreValid(group.section,
                                                        detail_outputs);
  MarkInputsInvalid(group.section, validity, validation_type == VALIDATE_FINAL);

  return !validity.HasErrors();
}
