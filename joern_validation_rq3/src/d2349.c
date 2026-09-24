Document* BaseAudioContext::GetDocument() const {
  return To<Document>(GetExecutionContext());
}
