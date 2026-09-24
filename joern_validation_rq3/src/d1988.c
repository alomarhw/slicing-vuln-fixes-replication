IntPoint PaintLayerScrollableArea::LastKnownMousePosition() const {
  return GetLayoutBox()->GetFrame() ? GetLayoutBox()
                                          ->GetFrame()
                                          ->GetEventHandler()
                                          .LastKnownMousePositionInRootFrame()
                                    : IntPoint();
}
