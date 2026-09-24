bool PrintWebViewHelper::RenderPagesForPrint(
    WebKit::WebFrame* frame,
    const WebKit::WebNode& node,
    PrepareFrameAndViewForPrint* prepare) {
  PrintMsg_PrintPages_Params print_settings = *print_pages_params_;
  if (print_settings.params.selection_only) {
    return CopyAndPrint(frame);
  } else {
    return PrintPages(print_settings, frame, node, prepare);
  }
}
