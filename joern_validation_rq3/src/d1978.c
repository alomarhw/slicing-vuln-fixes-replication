void LayoutBlockFlow::markAllDescendantsWithFloatsForLayout(LayoutBox* floatToRemove, bool inLayout)
{
    if (!everHadLayout() && !containsFloats())
        return;

    if (m_descendantsWithFloatsMarkedForLayout && !floatToRemove)
        return;
    m_descendantsWithFloatsMarkedForLayout |= !floatToRemove;

    MarkingBehavior markParents = inLayout ? MarkOnlyThis : MarkContainerChain;
    setChildNeedsLayout(markParents);

    if (floatToRemove)
        removeFloatingObject(floatToRemove);

    if (!childrenInline() || floatToRemove) {
        for (LayoutObject* child = firstChild(); child; child = child->nextSibling()) {
            if ((!floatToRemove && child->isFloatingOrOutOfFlowPositioned()) || !child->isLayoutBlock())
                continue;
            if (!child->isLayoutBlockFlow()) {
                LayoutBlock* childBlock = toLayoutBlock(child);
                if (childBlock->shrinkToAvoidFloats() && childBlock->everHadLayout())
                    childBlock->setChildNeedsLayout(markParents);
                continue;
            }
            LayoutBlockFlow* childBlockFlow = toLayoutBlockFlow(child);
            if ((floatToRemove ? childBlockFlow->containsFloat(floatToRemove) : childBlockFlow->containsFloats()) || childBlockFlow->shrinkToAvoidFloats())
                childBlockFlow->markAllDescendantsWithFloatsForLayout(floatToRemove, inLayout);
        }
    }
}
