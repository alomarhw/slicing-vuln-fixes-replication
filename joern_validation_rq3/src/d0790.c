void RenderBlock::fitBorderToLinesIfNeeded()
{
    if (style()->borderFit() == BorderFitBorder || hasOverrideWidth())
        return;

    LayoutUnit left = LayoutUnit::max();
    LayoutUnit right = LayoutUnit::min();
    LayoutUnit oldWidth = contentWidth();
    adjustForBorderFit(0, left, right);

    LayoutUnit leftEdge = borderLeft() + paddingLeft();
    LayoutUnit rightEdge = leftEdge + oldWidth;
    left = min(rightEdge, max(leftEdge, left));
    right = max(left, min(rightEdge, right));

    LayoutUnit newContentWidth = right - left;
    if (newContentWidth == oldWidth)
        return;

    setOverrideLogicalContentWidth(newContentWidth);
    layoutBlock(false);
    clearOverrideLogicalContentWidth();
}
