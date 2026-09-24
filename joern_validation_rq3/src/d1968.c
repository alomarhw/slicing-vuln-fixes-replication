void PageHandler::Navigate(const std::string& url,
                           Maybe<std::string> referrer,
                           Maybe<std::string> maybe_transition_type,
                           std::unique_ptr<NavigateCallback> callback) {
  GURL gurl(url);
  if (!gurl.is_valid()) {
    callback->sendFailure(Response::Error("Cannot navigate to invalid URL"));
    return;
  }

  WebContentsImpl* web_contents = GetWebContents();
  if (!web_contents) {
    callback->sendFailure(Response::InternalError());
    return;
  }

  ui::PageTransition type;
  std::string transition_type =
      maybe_transition_type.fromMaybe(Page::TransitionTypeEnum::Typed);
  if (transition_type == Page::TransitionTypeEnum::Link)
    type = ui::PAGE_TRANSITION_LINK;
  else if (transition_type == Page::TransitionTypeEnum::Typed)
    type = ui::PAGE_TRANSITION_TYPED;
  else if (transition_type == Page::TransitionTypeEnum::Auto_bookmark)
    type = ui::PAGE_TRANSITION_AUTO_BOOKMARK;
  else if (transition_type == Page::TransitionTypeEnum::Auto_subframe)
    type = ui::PAGE_TRANSITION_AUTO_SUBFRAME;
  else if (transition_type == Page::TransitionTypeEnum::Manual_subframe)
    type = ui::PAGE_TRANSITION_MANUAL_SUBFRAME;
  else if (transition_type == Page::TransitionTypeEnum::Generated)
    type = ui::PAGE_TRANSITION_GENERATED;
  else if (transition_type == Page::TransitionTypeEnum::Auto_toplevel)
    type = ui::PAGE_TRANSITION_AUTO_TOPLEVEL;
  else if (transition_type == Page::TransitionTypeEnum::Form_submit)
    type = ui::PAGE_TRANSITION_FORM_SUBMIT;
  else if (transition_type == Page::TransitionTypeEnum::Reload)
    type = ui::PAGE_TRANSITION_RELOAD;
  else if (transition_type == Page::TransitionTypeEnum::Keyword)
    type = ui::PAGE_TRANSITION_KEYWORD;
  else if (transition_type == Page::TransitionTypeEnum::Keyword_generated)
    type = ui::PAGE_TRANSITION_KEYWORD_GENERATED;
  else
    type = ui::PAGE_TRANSITION_TYPED;

  web_contents->GetController().LoadURL(
      gurl,
      Referrer(GURL(referrer.fromMaybe("")), blink::kWebReferrerPolicyDefault),
      type, std::string());
  if (IsBrowserSideNavigationEnabled()) {
    std::string frame_id =
        web_contents->GetMainFrame()->GetDevToolsFrameToken().ToString();
    if (navigate_callback_) {
      std::string error_string = net::ErrorToString(net::ERR_ABORTED);
      navigate_callback_->sendSuccess(frame_id, Maybe<std::string>(),
                                      Maybe<std::string>(error_string));
    }
    if (web_contents->GetMainFrame()->frame_tree_node()->navigation_request())
      navigate_callback_ = std::move(callback);
    else
      callback->sendSuccess(frame_id, Maybe<std::string>(),
                            Maybe<std::string>());
    return;
  }
  callback->fallThrough();
}
