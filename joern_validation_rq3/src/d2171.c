void RenderWidgetHostImpl::Redo() {
  Send(new ViewMsg_Redo(GetRoutingID()));
  RecordAction(UserMetricsAction("Redo"));
}
