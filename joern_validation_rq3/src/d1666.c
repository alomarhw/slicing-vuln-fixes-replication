void ResourceDispatcherHostImpl::CancelRequest(int child_id,
                                               int request_id) {
  ResourceLoader* loader = GetLoader(child_id, request_id);
  if (!loader) {
    DVLOG(1) << "Canceling a request that wasn't found";
    return;
  }

  RemovePendingRequest(child_id, request_id);
}
