ResourceLoadPriority TypeToPriority(Resource::Type type) {
  switch (type) {
    case Resource::kMainResource:
    case Resource::kCSSStyleSheet:
    case Resource::kFont:
      return kResourceLoadPriorityVeryHigh;
    case Resource::kXSLStyleSheet:
      DCHECK(RuntimeEnabledFeatures::XSLTEnabled());
    case Resource::kRaw:
    case Resource::kImportResource:
    case Resource::kScript:
      return kResourceLoadPriorityHigh;
    case Resource::kManifest:
    case Resource::kMock:
      return kResourceLoadPriorityMedium;
    case Resource::kImage:
    case Resource::kTextTrack:
    case Resource::kMedia:
    case Resource::kSVGDocument:
      return kResourceLoadPriorityLow;
    case Resource::kLinkPrefetch:
      return kResourceLoadPriorityVeryLow;
  }

  NOTREACHED();
  return kResourceLoadPriorityUnresolved;
}
