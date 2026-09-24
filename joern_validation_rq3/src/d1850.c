void RenderFrameImpl::didFinishLoad(blink::WebLocalFrame* frame) {
  DCHECK(!frame_ || frame_ == frame);
  WebDataSource* ds = frame->dataSource();
  DocumentState* document_state = DocumentState::FromDataSource(ds);
  if (document_state->finish_load_time().is_null()) {
    if (!frame->parent()) {
      TRACE_EVENT_INSTANT0("WebCore", "LoadFinished",
                           TRACE_EVENT_SCOPE_PROCESS);
    }
    document_state->set_finish_load_time(Time::Now());
  }

  FOR_EACH_OBSERVER(RenderViewObserver, render_view_->observers(),
                    DidFinishLoad(frame));
  FOR_EACH_OBSERVER(RenderFrameObserver, observers_, DidFinishLoad());

  if (is_swapped_out())
    return;

  Send(new FrameHostMsg_DidFinishLoad(routing_id_,
                                      ds->request().url()));
}
