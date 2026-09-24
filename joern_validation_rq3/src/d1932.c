void RenderBox::addOverflowFromChild(RenderBox* child, const LayoutSize& delta)
{
    if (child->isRenderFlowThread())
        return;

    LayoutRect childLayoutOverflowRect = child->layoutOverflowRectForPropagation(style());
    childLayoutOverflowRect.move(delta);
    addLayoutOverflow(childLayoutOverflowRect);

    if (child->hasSelfPaintingLayer())
        return;
    LayoutRect childVisualOverflowRect = child->visualOverflowRectForPropagation(style());
    childVisualOverflowRect.move(delta);
    addContentsVisualOverflow(childVisualOverflowRect);
}
