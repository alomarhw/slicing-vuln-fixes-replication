static bool NeedsFilter(const LayoutObject& object) {
  if (object.IsBoxModelObject() && ToLayoutBoxModelObject(object).Layer() &&
      (object.StyleRef().HasFilter() || object.HasReflection()))
    return true;
  if (object.IsLayoutImage() && ToLayoutImage(object).ShouldInvertColor())
    return true;
  return false;
}
