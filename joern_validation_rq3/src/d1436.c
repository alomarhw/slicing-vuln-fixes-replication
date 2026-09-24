void RenderBlockFlow::layoutBlock(bool relayoutChildren)
{
    ASSERT(needsLayout());
    ASSERT(isInlineBlockOrInlineTable() || !isInline());

    m_hasOnlySelfCollapsingChildren = false;

    if (!relayoutChildren && simplifiedLayout())
        return;

    SubtreeLayoutScope layoutScope(*this);

    bool done = false;
    LayoutUnit pageLogicalHeight = 0;
    LayoutRepainter repainter(*this, checkForRepaintDuringLayout());
    while (!done)
        done = layoutBlockFlow(relayoutChildren, pageLogicalHeight, layoutScope);

    fitBorderToLinesIfNeeded();

    RenderView* renderView = view();
    if (renderView->layoutState()->pageLogicalHeight())
        setPageLogicalOffset(renderView->layoutState()->pageLogicalOffset(*this, logicalTop()));

    updateLayerTransform();

    updateScrollInfoAfterLayout();

    bool didFullRepaint = repainter.repaintAfterLayout();
    if (!didFullRepaint && m_repaintLogicalTop != m_repaintLogicalBottom && (style()->visibility() == VISIBLE || enclosingLayer()->hasVisibleContent())) {
        if (RuntimeEnabledFeatures::repaintAfterLayoutEnabled())
            setShouldRepaintOverflow(true);
        else
            repaintOverflow();
    }
    clearNeedsLayout();
}
