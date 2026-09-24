void OmniboxEditModel::OnChanged() {
  const AutocompleteMatch& current_match = user_input_in_progress_ ?
      CurrentMatch(NULL) : AutocompleteMatch();

  AutocompleteActionPredictor::Action recommended_action =
      AutocompleteActionPredictor::ACTION_NONE;
  if (user_input_in_progress_) {
    InstantSearchPrerenderer* prerenderer =
        InstantSearchPrerenderer::GetForProfile(profile_);
    if (prerenderer &&
        prerenderer->IsAllowed(current_match, controller_->GetWebContents()) &&
        popup_model()->IsOpen() && has_focus()) {
      recommended_action = AutocompleteActionPredictor::ACTION_PRERENDER;
    } else {
      AutocompleteActionPredictor* action_predictor =
          predictors::AutocompleteActionPredictorFactory::GetForProfile(
              profile_);
      action_predictor->RegisterTransitionalMatches(user_text_, result());
      recommended_action =
          action_predictor->RecommendAction(user_text_, current_match);
    }
  }

  UMA_HISTOGRAM_ENUMERATION("AutocompleteActionPredictor.Action",
                            recommended_action,
                            AutocompleteActionPredictor::LAST_PREDICT_ACTION);

  view_->SetGrayTextAutocompletion(base::string16());

  switch (recommended_action) {
    case AutocompleteActionPredictor::ACTION_PRERENDER:
      if (!delegate_->CurrentPageExists())
        break;
      if (current_match.destination_url != delegate_->GetURL())
        delegate_->DoPrerender(current_match);
      break;
    case AutocompleteActionPredictor::ACTION_PRECONNECT:
      omnibox_controller_->DoPreconnect(current_match);
      break;
    case AutocompleteActionPredictor::ACTION_NONE:
      break;
  }

  controller_->OnChanged();
}
