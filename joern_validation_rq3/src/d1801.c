void AudioContext::derefUnfinishedSourceNodes()
{
    ASSERT(isMainThread());
    for (unsigned i = 0; i < m_referencedNodes.size(); ++i)
        m_referencedNodes.at(i)->breakConnection();

    m_referencedNodes.clear();
}
