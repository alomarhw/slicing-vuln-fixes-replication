void AudioContext::registerLiveNode(AudioNode& node)
{
    ASSERT(isMainThread());
    m_liveNodes.add(&node, adoptPtr(new AudioNodeDisposer(node)));
}
