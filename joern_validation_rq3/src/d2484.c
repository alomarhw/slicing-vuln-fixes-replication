void InspectorResourceAgent::didFailLoading(unsigned long identifier, const ResourceError& error)
{
    String requestId = IdentifiersFactory::requestId(identifier);
    bool canceled = error.isCancellation();
    m_frontend->loadingFailed(requestId, currentTime(), error.localizedDescription(), canceled ? &canceled : 0);
}
