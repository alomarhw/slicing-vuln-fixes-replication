LayoutUnit RenderBox::computeContentBoxLogicalWidth(LayoutUnit width) const
{
    if (style()->boxSizing() == BORDER_BOX)
        width -= borderAndPaddingLogicalWidth();
    return max(0, width);
}
