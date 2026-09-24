bool AXLayoutObject::isTabItemSelected() const {
  if (!isTabItem() || !getLayoutObject())
    return false;

  Node* node = getNode();
  if (!node || !node->isElementNode())
    return false;

  AXObject* focusedElement = axObjectCache().focusedObject();
  if (!focusedElement)
    return false;

  HeapVector<Member<Element>> elements;
  elementsFromAttribute(elements, aria_controlsAttr);

  for (const auto& element : elements) {
    AXObject* tabPanel = axObjectCache().getOrCreate(element);

    if (!tabPanel || tabPanel->roleValue() != TabPanelRole)
      continue;

    AXObject* checkFocusElement = focusedElement;
    while (checkFocusElement) {
      if (tabPanel == checkFocusElement)
        return true;
      checkFocusElement = checkFocusElement->parentObject();
    }
  }

  return false;
}
