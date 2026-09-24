void RenderViewHostImpl::OnTargetDropACK() {
  NotificationService::current()->Notify(
      NOTIFICATION_RENDER_VIEW_HOST_DID_RECEIVE_DRAG_TARGET_DROP_ACK,
      Source<RenderViewHost>(this),
      NotificationService::NoDetails());
}
