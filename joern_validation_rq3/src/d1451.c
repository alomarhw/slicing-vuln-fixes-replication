bool HpackDecoder::DecodeNextName(
    HpackInputStream* input_stream, StringPiece* next_name) {
  uint32 index_or_zero = 0;
  if (!input_stream->DecodeNextUint32(&index_or_zero))
    return false;

  if (index_or_zero == 0)
    return DecodeNextStringLiteral(input_stream, true, next_name);

  const HpackEntry* entry = header_table_.GetByIndex(index_or_zero);
  if (entry == NULL) {
    return false;
  } else if (entry->IsStatic()) {
    *next_name = entry->name();
  } else {
    key_buffer_.assign(entry->name());
    *next_name = key_buffer_;
  }
  return true;
}
