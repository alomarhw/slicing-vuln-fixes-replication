uint32_t PDFiumEngine::GetCharUnicode(int page_index, int char_index) {
  DCHECK(page_index >= 0 && page_index < static_cast<int>(pages_.size()));
  return pages_[page_index]->GetCharUnicode(char_index);
}
