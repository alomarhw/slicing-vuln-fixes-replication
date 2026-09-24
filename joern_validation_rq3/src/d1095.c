size_t UrlData::CachedSize() {
  DCHECK(thread_checker_.CalledOnValidThread());
  return multibuffer()->map().size();
}
