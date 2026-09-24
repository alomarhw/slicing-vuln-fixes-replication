void FilePathWatcherImpl::OnFilePathChanged(
    InotifyReader::Watch fired_watch,
    const FilePath::StringType& child,
    bool created,
    bool is_directory) {

  if (!message_loop()->BelongsToCurrentThread()) {
    message_loop()->PostTask(FROM_HERE,
        base::Bind(&FilePathWatcherImpl::OnFilePathChanged,
                   this,
                   fired_watch,
                   child,
                   created,
                   is_directory));
    return;
  }

  DCHECK(MessageLoopForIO::current());

  WatchVector::const_iterator watch_entry(watches_.begin());
  for ( ; watch_entry != watches_.end(); ++watch_entry) {
    if (fired_watch == watch_entry->watch_) {
      bool change_on_target_path = child.empty() ||
          ((child == watch_entry->subdir_) && watch_entry->linkname_.empty()) ||
          (child == watch_entry->linkname_);

      DCHECK(watch_entry->subdir_.empty() ||
          (watch_entry + 1) != watches_.end());
      bool target_changed =
          (watch_entry->subdir_.empty() && (child == watch_entry->linkname_)) ||
          (watch_entry->subdir_.empty() && watch_entry->linkname_.empty()) ||
          (watch_entry->subdir_ == child && (watch_entry + 1)->subdir_.empty());

      if (change_on_target_path && !UpdateWatches()) {
        delegate_->OnFilePathError(target_);
        return;
      }

      if (target_changed ||
          (change_on_target_path && !created) ||
          (change_on_target_path && file_util::PathExists(target_))) {
        delegate_->OnFilePathChanged(target_);
        return;
      }
    }
  }

}
