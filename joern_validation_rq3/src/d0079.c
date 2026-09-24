CompositingReasons RenderLayerCompositor::subtreeReasonsForCompositing(RenderObject* renderer, bool hasCompositedDescendants, bool has3DTransformedDescendants) const
{
    CompositingReasons subtreeReasons = CompositingReasonNone;

    RenderLayer* layer = toRenderBoxModelObject(renderer)->layer();

    if (hasCompositedDescendants) {
        if (layer->transform())
            subtreeReasons |= CompositingReasonTransformWithCompositedDescendants;

        if (layer->shouldIsolateCompositedDescendants()) {
            ASSERT(layer->stackingNode()->isStackingContext());
            subtreeReasons |= CompositingReasonIsolateCompositedDescendants;
        }

        ASSERT((renderer->isTransparent() || renderer->hasMask() || renderer->hasFilter() || renderer->hasBlendMode()) == renderer->createsGroup());
        if (renderer->isTransparent())
            subtreeReasons |= CompositingReasonOpacityWithCompositedDescendants;
        if (renderer->hasMask())
            subtreeReasons |= CompositingReasonMaskWithCompositedDescendants;
        if (renderer->hasFilter())
            subtreeReasons |= CompositingReasonFilterWithCompositedDescendants;
        if (renderer->hasBlendMode())
            subtreeReasons |= CompositingReasonBlendingWithCompositedDescendants;

        if (renderer->hasReflection())
            subtreeReasons |= CompositingReasonReflectionWithCompositedDescendants;

        if (renderer->hasClipOrOverflowClip())
            subtreeReasons |= CompositingReasonClipsCompositingDescendants;
    }


    if (has3DTransformedDescendants) {
        if (renderer->style()->transformStyle3D() == TransformStyle3DPreserve3D)
            subtreeReasons |= CompositingReasonPreserve3D;

        if (renderer->style()->hasPerspective())
            subtreeReasons |= CompositingReasonPerspective;
    }

    return subtreeReasons;
}
