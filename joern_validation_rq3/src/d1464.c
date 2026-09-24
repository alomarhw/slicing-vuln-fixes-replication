static bool isMenuListOption(Node* node) {
  if (!isHTMLOptionElement(node))
    return false;
  HTMLSelectElement* select = toHTMLOptionElement(node)->ownerSelectElement();
  if (!select)
    return false;
  LayoutObject* layoutObject = select->layoutObject();
  return layoutObject && layoutObject->isMenuList();
}
