void ResourceFetcher::StorePerformanceTimingInitiatorInformation(
    Resource* resource) {
  const AtomicString& fetch_initiator = resource->Options().initiator_info.name;
  if (fetch_initiator == FetchInitiatorTypeNames::internal)
    return;

  bool is_main_resource = resource->GetType() == Resource::kMainResource;

  double start_time = resource->GetResourceRequest().NavigationStartTime()
                          ? resource->GetResourceRequest().NavigationStartTime()
                          : MonotonicallyIncreasingTime();

  if (is_main_resource) {
    DCHECK(!navigation_timing_info_);
    navigation_timing_info_ = ResourceTimingInfo::Create(
        fetch_initiator, start_time, is_main_resource);
  }

  RefPtr<ResourceTimingInfo> info =
      ResourceTimingInfo::Create(fetch_initiator, start_time, is_main_resource);

  if (resource->IsCacheValidator()) {
    const AtomicString& timing_allow_origin =
        resource->GetResponse().HttpHeaderField(HTTPNames::Timing_Allow_Origin);
    if (!timing_allow_origin.IsEmpty())
      info->SetOriginalTimingAllowOrigin(timing_allow_origin);
  }

  if (!is_main_resource ||
      Context().UpdateTimingInfoForIFrameNavigation(info.Get())) {
    resource_timing_info_map_.insert(resource, info.Release());
  }
}
