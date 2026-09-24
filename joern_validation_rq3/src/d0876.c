Status IndexedDBDatabase::GetOperation(
    int64_t object_store_id,
    int64_t index_id,
    std::unique_ptr<IndexedDBKeyRange> key_range,
    indexed_db::CursorType cursor_type,
    scoped_refptr<IndexedDBCallbacks> callbacks,
    IndexedDBTransaction* transaction) {
  IDB_TRACE1("IndexedDBDatabase::GetOperation", "txn.id", transaction->id());

  DCHECK(metadata_.object_stores.find(object_store_id) !=
         metadata_.object_stores.end());
  const IndexedDBObjectStoreMetadata& object_store_metadata =
      metadata_.object_stores[object_store_id];

  const IndexedDBKey* key;

  Status s = Status::OK();
  std::unique_ptr<IndexedDBBackingStore::Cursor> backing_store_cursor;
  if (key_range->IsOnlyKey()) {
    key = &key_range->lower();
  } else {
    if (index_id == IndexedDBIndexMetadata::kInvalidId) {
      if (cursor_type == indexed_db::CURSOR_KEY_ONLY) {
        backing_store_cursor = backing_store_->OpenObjectStoreKeyCursor(
            transaction->BackingStoreTransaction(), id(), object_store_id,
            *key_range, blink::mojom::IDBCursorDirection::Next, &s);
      } else {
        backing_store_cursor = backing_store_->OpenObjectStoreCursor(
            transaction->BackingStoreTransaction(), id(), object_store_id,
            *key_range, blink::mojom::IDBCursorDirection::Next, &s);
      }
    } else if (cursor_type == indexed_db::CURSOR_KEY_ONLY) {
      backing_store_cursor = backing_store_->OpenIndexKeyCursor(
          transaction->BackingStoreTransaction(), id(), object_store_id,
          index_id, *key_range, blink::mojom::IDBCursorDirection::Next, &s);
    } else {
      backing_store_cursor = backing_store_->OpenIndexCursor(
          transaction->BackingStoreTransaction(), id(), object_store_id,
          index_id, *key_range, blink::mojom::IDBCursorDirection::Next, &s);
    }

    if (!s.ok())
      return s;

    if (!backing_store_cursor) {
      callbacks->OnSuccess();
      return s;
    }

    key = &backing_store_cursor->key();
  }

  std::unique_ptr<IndexedDBKey> primary_key;
  if (index_id == IndexedDBIndexMetadata::kInvalidId) {
    IndexedDBReturnValue value;
    s = backing_store_->GetRecord(transaction->BackingStoreTransaction(),
                                  id(),
                                  object_store_id,
                                  *key,
                                  &value);
    if (!s.ok())
      return s;

    if (value.empty()) {
      callbacks->OnSuccess();
      return s;
    }

    if (cursor_type == indexed_db::CURSOR_KEY_ONLY) {
      callbacks->OnSuccess(*key);
      return s;
    }

    if (object_store_metadata.auto_increment &&
        !object_store_metadata.key_path.IsNull()) {
      value.primary_key = *key;
      value.key_path = object_store_metadata.key_path;
    }

    callbacks->OnSuccess(&value);
    return s;
  }

  s = backing_store_->GetPrimaryKeyViaIndex(
      transaction->BackingStoreTransaction(),
      id(),
      object_store_id,
      index_id,
      *key,
      &primary_key);
  if (!s.ok())
    return s;

  if (!primary_key) {
    callbacks->OnSuccess();
    return s;
  }
  if (cursor_type == indexed_db::CURSOR_KEY_ONLY) {
    callbacks->OnSuccess(*primary_key);
    return s;
  }

  IndexedDBReturnValue value;
  s = backing_store_->GetRecord(transaction->BackingStoreTransaction(),
                                id(),
                                object_store_id,
                                *primary_key,
                                &value);
  if (!s.ok())
    return s;

  if (value.empty()) {
    callbacks->OnSuccess();
    return s;
  }
  if (object_store_metadata.auto_increment &&
      !object_store_metadata.key_path.IsNull()) {
    value.primary_key = *primary_key;
    value.key_path = object_store_metadata.key_path;
  }
  callbacks->OnSuccess(&value);
  return s;
}
