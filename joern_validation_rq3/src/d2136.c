void WebPageProxy::getWindowFrame(FloatRect& newWindowFrame)
{
    newWindowFrame = m_pageClient->convertToUserSpace(m_uiClient.windowFrame(this));
}
