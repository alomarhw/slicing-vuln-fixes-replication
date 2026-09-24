void OfflinePageModelImpl::DeletePagesByOfflineId(
    const std::vector<int64_t>& offline_ids,
    const DeletePageCallback& callback) {
  RunWhenLoaded(base::Bind(&OfflinePageModelImpl::DoDeletePagesByOfflineId,
                           weak_ptr_factory_.GetWeakPtr(), offline_ids,
                           callback));
}
