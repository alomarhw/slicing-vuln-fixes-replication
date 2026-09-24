bool IsTextTooLongAt(const Position& position) {
  const Element* element = EnclosingTextControl(position);
  if (!element)
    return false;
  if (isHTMLInputElement(element))
    return toHTMLInputElement(element)->TooLong();
  if (isHTMLTextAreaElement(element))
    return toHTMLTextAreaElement(element)->TooLong();
  return false;
}
