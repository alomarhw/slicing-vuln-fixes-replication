 void TypingCommand::adjustSelectionAfterIncrementalInsertion(
    LocalFrame* frame,
    const size_t textLength) {
  if (!isIncrementalInsertion())
    return;

  frame->document()->updateStyleAndLayoutIgnorePendingStylesheets();

  Element* element = frame->selection()
                         .computeVisibleSelectionInDOMTreeDeprecated()
                         .rootEditableElement();
  DCHECK(element);

  const size_t end = m_selectionStart + textLength;
  const size_t start =
      compositionType() == TextCompositionUpdate ? m_selectionStart : end;
  const SelectionInDOMTree& selection =
      createSelection(start, end, endingSelection().isDirectional(), element);

  if (selection ==
      frame->selection()
          .computeVisibleSelectionInDOMTreeDeprecated()
          .asSelection())
    return;

  setEndingSelection(selection);
  frame->selection().setSelection(selection);
}
