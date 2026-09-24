void DelegatedFrameHost::SendReturnedDelegatedResources(
    uint32 output_surface_id) {
  RenderWidgetHostImpl* host = client_->GetHost();

  cc::CompositorFrameAck ack;
  if (!surface_returned_resources_.empty()) {
    ack.resources.swap(surface_returned_resources_);
  } else {
    DCHECK(resource_collection_);
    resource_collection_->TakeUnusedResourcesForChildCompositor(&ack.resources);
  }
  DCHECK(!ack.resources.empty());

  RenderWidgetHostImpl::SendReclaimCompositorResources(
      host->GetRoutingID(),
      output_surface_id,
      host->GetProcess()->GetID(),
      ack);
}
