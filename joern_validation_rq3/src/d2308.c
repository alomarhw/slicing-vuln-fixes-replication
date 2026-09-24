void WebPageProxy::setUserAgent(const String& userAgent)
{
    if (m_userAgent == userAgent)
        return;
    m_userAgent = userAgent;

    if (!isValid())
        return;
    process()->send(Messages::WebPage::SetUserAgent(m_userAgent), m_pageID);
}
