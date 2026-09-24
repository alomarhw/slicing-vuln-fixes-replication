void InspectorAgentRegistry::registerInDispatcher(InspectorBackendDispatcher* dispatcher)
{
    for (size_t i = 0; i < m_agents.size(); i++)
        m_agents[i]->registerInDispatcher(dispatcher);
}
