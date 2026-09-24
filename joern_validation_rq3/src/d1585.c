void WebDevToolsAgentImpl::enableTracing(const String& categoryFilter)
{
    m_client->enableTracing(categoryFilter);
}
