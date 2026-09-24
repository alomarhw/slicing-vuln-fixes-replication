void RTCPeerConnection::didChangeReadyState(ReadyState newState)
{
    ASSERT(scriptExecutionContext()->isContextThread());
    changeReadyState(newState);
}
