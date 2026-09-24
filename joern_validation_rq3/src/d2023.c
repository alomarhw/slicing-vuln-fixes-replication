Node* WebPagePrivate::nodeForZoomUnderPoint(const IntPoint& documentPoint)
{
    if (!m_mainFrame)
        return 0;

    HitTestResult result = m_mainFrame->eventHandler()->hitTestResultAtPoint(documentPoint, false);

    Node* node = result.innerNonSharedNode();

    if (!node)
        return 0;

    RenderObject* renderer = node->renderer();
    while (!renderer) {
        node = node->parentNode();
        renderer = node->renderer();
    }

    return node;
}
