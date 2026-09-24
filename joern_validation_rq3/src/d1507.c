void WebPluginProxy::UpdateIMEStatus() {
  int input_type;
  gfx::Rect caret_rect;
  if (!delegate_->GetIMEStatus(&input_type, &caret_rect))
    return;

  Send(new PluginHostMsg_NotifyIMEStatus(route_id_, input_type, caret_rect));
}
