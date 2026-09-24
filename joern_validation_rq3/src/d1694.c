void WebPageProxy::scrollView(const IntRect& scrollRect, const IntSize& scrollOffset)
{
    m_pageClient->scrollView(scrollRect, scrollOffset);
}
