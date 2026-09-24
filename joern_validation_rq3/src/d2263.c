void DownloadItemImpl::Remove() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  delegate_->AssertStateConsistent(this);
  Cancel(true);
  delegate_->AssertStateConsistent(this);

  TransitionTo(REMOVING);
  delegate_->DownloadRemoved(this);
}
