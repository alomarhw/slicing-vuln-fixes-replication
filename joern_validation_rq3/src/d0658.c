void InspectorResourceAgent::didReceiveWebSocketFrameError(unsigned long identifier, const String& errorMessage)
{
    m_frontend->webSocketFrameError(IdentifiersFactory::requestId(identifier), currentTime(), errorMessage);
}
