void SVGLayoutSupport::intersectPaintInvalidationRectWithResources(const LayoutObject* layoutObject, FloatRect& paintInvalidationRect)
{
    ASSERT(layoutObject);

    SVGResources* resources = SVGResourcesCache::cachedResourcesForLayoutObject(layoutObject);
    if (!resources)
        return;

    if (LayoutSVGResourceFilter* filter = resources->filter())
        paintInvalidationRect = filter->resourceBoundingBox(layoutObject);

    if (LayoutSVGResourceClipper* clipper = resources->clipper())
        paintInvalidationRect.intersect(clipper->resourceBoundingBox(layoutObject));

    if (LayoutSVGResourceMasker* masker = resources->masker())
        paintInvalidationRect.intersect(masker->resourceBoundingBox(layoutObject));
}
