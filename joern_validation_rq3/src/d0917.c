void WebDevToolsAgentImpl::flushPendingFrontendMessages()
{
    InspectorController* ic = inspectorController();
    ic->flushPendingFrontendMessages();

    for (size_t i = 0; i < m_frontendMessageQueue.size(); ++i)
        m_client->sendMessageToInspectorFrontend(m_frontendMessageQueue[i]->toJSONString());
    m_frontendMessageQueue.clear();
}
