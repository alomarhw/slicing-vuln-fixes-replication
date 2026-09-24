void SessionModelAssociator::InitializeCurrentMachineTag(
    sync_api::WriteTransaction* trans) {
  DCHECK(CalledOnValidThread());
  syncable::Directory* dir = trans->GetWrappedWriteTrans()->directory();
  current_machine_tag_ = "session_sync";
  current_machine_tag_.append(dir->cache_guid());
  VLOG(1) << "Creating machine tag: " << current_machine_tag_;
  tab_pool_.set_machine_tag(current_machine_tag_);
}
