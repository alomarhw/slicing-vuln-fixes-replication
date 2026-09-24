void QQuickWebView::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(QQuickWebView);
    d->pageView->eventHandler()->handleMouseReleaseEvent(event);
}
