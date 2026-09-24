void IndexedDBDatabase::RenameObjectStore(IndexedDBTransaction* transaction,
                                          int64_t object_store_id,
                                          const base::string16& new_name) {
  DCHECK(transaction);
  IDB_TRACE1("IndexedDBDatabase::RenameObjectStore", "txn.id",
             transaction->id());
  DCHECK_EQ(transaction->mode(), blink::kWebIDBTransactionModeVersionChange);

  if (!ValidateObjectStoreId(object_store_id))
    return;

  IndexedDBObjectStoreMetadata& object_store_metadata =
      metadata_.object_stores[object_store_id];

  base::string16 old_name;

  Status s = metadata_coding_->RenameObjectStore(
      transaction->BackingStoreTransaction()->transaction(),
      transaction->database()->id(), new_name, &old_name,
      &object_store_metadata);

  if (!s.ok()) {
    ReportErrorWithDetails(s, "Internal error renaming object store.");
    return;
  }
  DCHECK_EQ(object_store_metadata.name, new_name);

  transaction->ScheduleAbortTask(
      base::BindOnce(&IndexedDBDatabase::RenameObjectStoreAbortOperation, this,
                     object_store_id, std::move(old_name)));
}
