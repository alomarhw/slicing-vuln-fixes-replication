HTMLFormControlElement* HTMLFormControlElement::enclosingFormControlElement(
    Node* node) {
  if (!node)
    return nullptr;
  return Traversal<HTMLFormControlElement>::firstAncestorOrSelf(*node);
}
