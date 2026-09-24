void RenderBox::paintBackgroundWithBorderAndBoxShadow(PaintInfo& paintInfo, const LayoutRect& paintRect, BackgroundBleedAvoidance bleedAvoidance)
{
    IntRect snappedPaintRect(pixelSnappedIntRect(paintRect));
    bool themePainted = style()->hasAppearance() && !RenderTheme::theme().paint(this, paintInfo, snappedPaintRect);
    if (!themePainted) {
        if (bleedAvoidance == BackgroundBleedBackgroundOverBorder)
            paintBorder(paintInfo, paintRect, style(), bleedAvoidance);

        paintBackground(paintInfo, paintRect, bleedAvoidance);

        if (style()->hasAppearance())
            RenderTheme::theme().paintDecorations(this, paintInfo, snappedPaintRect);
    }
    paintBoxShadow(paintInfo, paintRect, style(), Inset);

    if (bleedAvoidance != BackgroundBleedBackgroundOverBorder && (!style()->hasAppearance() || (!themePainted && RenderTheme::theme().paintBorderOnly(this, paintInfo, snappedPaintRect))) && style()->hasBorder() && !(isTable() && toRenderTable(this)->collapseBorders()))
        paintBorder(paintInfo, paintRect, style(), bleedAvoidance);
}
