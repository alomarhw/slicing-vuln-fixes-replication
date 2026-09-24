void FrameLoader::LoadInSameDocument(
    const KURL& url,
    scoped_refptr<SerializedScriptValue> state_object,
    WebFrameLoadType frame_load_type,
    HistoryItem* history_item,
    ClientRedirectPolicy client_redirect,
    Document* initiating_document) {
  DCHECK(!state_object || frame_load_type == WebFrameLoadType::kBackForward);

  DetachDocumentLoader(provisional_document_loader_);

  if (!frame_->GetPage())
    return;
  SaveScrollState();

  KURL old_url = frame_->GetDocument()->Url();
  bool hash_change = EqualIgnoringFragmentIdentifier(url, old_url) &&
                     url.FragmentIdentifier() != old_url.FragmentIdentifier();
  if (hash_change) {
    frame_->GetEventHandler().StopAutoscroll();
    frame_->DomWindow()->EnqueueHashchangeEvent(old_url, url);
  }
  document_loader_->SetIsClientRedirect(client_redirect ==
                                        ClientRedirectPolicy::kClientRedirect);
  if (history_item)
    document_loader_->SetItemForHistoryNavigation(history_item);
  UpdateForSameDocumentNavigation(url, kSameDocumentNavigationDefault, nullptr,
                                  kScrollRestorationAuto, frame_load_type,
                                  initiating_document);

  ClearInitialScrollState();

  frame_->GetDocument()->CheckCompleted();

  std::unique_ptr<HistoryItem::ViewState> view_state;
  if (history_item && history_item->GetViewState()) {
    view_state =
        std::make_unique<HistoryItem::ViewState>(*history_item->GetViewState());
  }

  frame_->DomWindow()->StatePopped(state_object
                                       ? std::move(state_object)
                                       : SerializedScriptValue::NullValue());

  if (history_item) {
    RestoreScrollPositionAndViewState(
        frame_load_type, true /* is_same_document */, view_state.get(),
        history_item->ScrollRestorationType());
  }

  ProcessFragment(url, frame_load_type, kNavigationWithinSameDocument);

  TakeObjectSnapshot();
}
