bool MediaControlPanelElement::keepEventInNode(Event* event) {
  return isUserInteractionEvent(event);
}
