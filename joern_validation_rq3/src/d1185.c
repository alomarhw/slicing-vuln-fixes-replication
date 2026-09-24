void ResourceDispatcherHostImpl::CancelRequestsForRoute(int child_id,
                                                        int route_id) {

  bool any_requests_transferring = false;
  std::vector<GlobalRequestID> matching_requests;
  for (const auto& loader : pending_loaders_) {
    if (loader.first.child_id != child_id)
      continue;

    ResourceRequestInfoImpl* info = loader.second->GetRequestInfo();

    GlobalRequestID id(child_id, loader.first.request_id);
    DCHECK(id == loader.first);
    if (IsTransferredNavigation(id))
      any_requests_transferring = true;
    if (info->detachable_handler()) {
      info->detachable_handler()->Detach();
    } else if (!info->IsDownload() && !info->is_stream() &&
               !IsTransferredNavigation(id) &&
               (route_id == -1 || route_id == info->GetRouteID())) {
      matching_requests.push_back(id);
    }
  }

  for (size_t i = 0; i < matching_requests.size(); ++i) {
    LoaderMap::iterator iter = pending_loaders_.find(matching_requests[i]);
    if (iter != pending_loaders_.end())
      RemovePendingLoader(iter);
  }

  if (any_requests_transferring)
    return;

  if (route_id != -1) {
    if (blocked_loaders_map_.find(GlobalRoutingID(child_id, route_id)) !=
        blocked_loaders_map_.end()) {
      CancelBlockedRequestsForRoute(child_id, route_id);
    }
  } else {
    std::set<int> route_ids;
    for (const auto& blocked_loaders : blocked_loaders_map_) {
      if (blocked_loaders.first.child_id == child_id)
        route_ids.insert(blocked_loaders.first.route_id);
    }
    for (int route_id : route_ids) {
      CancelBlockedRequestsForRoute(child_id, route_id);
    }
  }
}
