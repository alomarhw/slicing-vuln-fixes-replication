void FrameLoader::ProcessFragment(const KURL& url,
                                  WebFrameLoadType frame_load_type,
                                  LoadStartType load_start_type) {
  LocalFrameView* view = frame_->View();
  if (!view)
    return;

  Frame* boundary_frame =
      url.HasFragmentIdentifier()
          ? frame_->FindUnsafeParentScrollPropagationBoundary()
          : nullptr;

  if (auto* boundary_local_frame = DynamicTo<LocalFrame>(boundary_frame))
    boundary_local_frame->View()->SetSafeToPropagateScrollToParent(false);

  bool should_scroll_to_fragment =
      (load_start_type == kNavigationWithinSameDocument &&
       !IsBackForwardLoadType(frame_load_type)) ||
      (!GetDocumentLoader()->GetInitialScrollState().did_restore_from_history &&
       !(GetDocumentLoader()->GetHistoryItem() &&
         GetDocumentLoader()->GetHistoryItem()->ScrollRestorationType() ==
             kScrollRestorationManual));

  view->ProcessUrlFragment(url, should_scroll_to_fragment);

  if (auto* boundary_local_frame = DynamicTo<LocalFrame>(boundary_frame))
    boundary_local_frame->View()->SetSafeToPropagateScrollToParent(true);
}
