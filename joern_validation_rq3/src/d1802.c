std::unique_ptr<Backend::Iterator> BackendImpl::CreateIterator() {
  return std::unique_ptr<Backend::Iterator>(
      new IteratorImpl(GetBackgroundQueue()));
}
