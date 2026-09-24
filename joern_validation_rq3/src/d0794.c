bool NormalPage::IsEmpty() {
  HeapObjectHeader* header = reinterpret_cast<HeapObjectHeader*>(Payload());
  return header->IsFree() && header->size() == PayloadSize();
}
