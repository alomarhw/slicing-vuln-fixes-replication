bool ResourceFetcher::IsFetching() const {
  return !loaders_.IsEmpty();
}
