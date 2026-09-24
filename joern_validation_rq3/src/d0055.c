FloatRect DragController::ClippedSelection(const LocalFrame& frame) {
  DCHECK(frame.View());
  return DataTransfer::ClipByVisualViewport(
      FloatRect(frame.Selection().UnclippedBoundsInDocument()), frame);
}
