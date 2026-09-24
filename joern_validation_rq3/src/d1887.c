static bool NeedsPaintOffsetTranslation(const LayoutObject& object) {
  if (!object.IsBoxModelObject())
    return false;

  if (object.IsSVGForeignObject())
    return false;

  const LayoutBoxModelObject& box_model = ToLayoutBoxModelObject(object);

  if (box_model.IsLayoutView()) {
    return true;
  }

  if (box_model.HasLayer() && box_model.Layer()->PaintsWithTransform(
                                  kGlobalPaintFlattenCompositingLayers)) {
    return true;
  }
  if (NeedsScrollOrScrollTranslation(object))
    return true;
  if (NeedsPaintOffsetTranslationForScrollbars(box_model))
    return true;
  if (NeedsSVGLocalToBorderBoxTransform(object))
    return true;

  if (!RuntimeEnabledFeatures::SlimmingPaintV2Enabled() &&
      (object.IsLayoutBlock() || object.IsLayoutReplaced()) &&
      object.HasLayer() &&
      !ToLayoutBoxModelObject(object).Layer()->EnclosingPaginationLayer() &&
      object.GetCompositingState() == kPaintsIntoOwnBacking)
    return true;

  return false;
}
