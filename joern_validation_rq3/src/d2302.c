void WebPagePrivate::clearDocumentData(const Document* documentGoingAway)
{
    ASSERT(documentGoingAway);
    if (m_currentContextNode && m_currentContextNode->document() == documentGoingAway)
        m_currentContextNode = 0;

    if (m_currentPinchZoomNode && m_currentPinchZoomNode->document() == documentGoingAway)
        m_currentPinchZoomNode = 0;

    if (m_currentBlockZoomAdjustedNode && m_currentBlockZoomAdjustedNode->document() == documentGoingAway)
        m_currentBlockZoomAdjustedNode = 0;

    if (m_inRegionScroller->d->isActive())
        m_inRegionScroller->d->clearDocumentData(documentGoingAway);

    if (documentGoingAway->frame())
        m_inputHandler->frameUnloaded(documentGoingAway->frame());

    Node* nodeUnderFatFinger = m_touchEventHandler->lastFatFingersResult().node();
    if (nodeUnderFatFinger && nodeUnderFatFinger->document() == documentGoingAway)
        m_touchEventHandler->resetLastFatFingersResult();

}
