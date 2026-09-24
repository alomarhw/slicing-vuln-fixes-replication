bool SVGLayoutSupport::isIsolationRequired(const LayoutObject* object)
{
    return willIsolateBlendingDescendantsForObject(object) && object->hasNonIsolatedBlendingDescendants();
}
