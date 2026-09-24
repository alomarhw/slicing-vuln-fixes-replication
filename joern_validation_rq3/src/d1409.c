void RenderFlexibleBox::updateAutoMarginsInMainAxis(RenderBox* child, LayoutUnit autoMarginOffset)
{
    ASSERT(autoMarginOffset >= 0);

    if (isHorizontalFlow()) {
        if (child->style()->marginLeft().isAuto())
            child->setMarginLeft(autoMarginOffset);
        if (child->style()->marginRight().isAuto())
            child->setMarginRight(autoMarginOffset);
    } else {
        if (child->style()->marginTop().isAuto())
            child->setMarginTop(autoMarginOffset);
        if (child->style()->marginBottom().isAuto())
            child->setMarginBottom(autoMarginOffset);
    }
}
