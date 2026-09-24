void PrintRenderFrameHelper::OnPrintPages() {
  if (ipc_nesting_level_ > 1)
    return;

  auto weak_this = weak_ptr_factory_.GetWeakPtr();
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  frame->DispatchBeforePrintEvent();
  if (!weak_this)
    return;

  auto plugin = delegate_->GetPdfElement(frame);
  Print(frame, plugin, false /* is_scripted? */);
  if (weak_this)
    frame->DispatchAfterPrintEvent();
}
