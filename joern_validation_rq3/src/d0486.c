void WebPagePrivate::zoomToContentRect(const IntRect& rect)
{
    if (!isUserScalable())
        return;

    FloatPoint anchor = FloatPoint(rect.width() / 2.0 + rect.x(), rect.height() / 2.0 + rect.y());
    IntSize viewSize = viewportSize();

    double scaleH = (double)viewSize.width() / (double)rect.width();
    double scaleV = (double)viewSize.height() / (double)rect.height();

    zoomAboutPoint(min(scaleH, scaleV), anchor);
}
