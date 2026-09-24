void BaseNode::SetUnencryptedSpecifics(
    const sync_pb::EntitySpecifics& specifics) {
  syncable::ModelType type = syncable::GetModelTypeFromSpecifics(specifics);
  DCHECK_NE(syncable::UNSPECIFIED, type);
  if (GetModelType() != syncable::UNSPECIFIED) {
    DCHECK_EQ(GetModelType(), type);
  }
  unencrypted_data_.CopyFrom(specifics);
}
