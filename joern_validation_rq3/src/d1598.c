void ResourceFetcher::StopFetching() {
  HeapVector<Member<ResourceLoader>> loaders_to_cancel;
  for (const auto& loader : non_blocking_loaders_)
    loaders_to_cancel.push_back(loader);
  for (const auto& loader : loaders_)
    loaders_to_cancel.push_back(loader);

  for (const auto& loader : loaders_to_cancel) {
    if (loaders_.Contains(loader) || non_blocking_loaders_.Contains(loader))
      loader->Cancel();
  }
}
