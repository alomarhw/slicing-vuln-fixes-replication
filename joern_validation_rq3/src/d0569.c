KURL FrameLoader::baseURL() const
{
    ASSERT(m_frame->document());
    return m_frame->document()->baseURL();
}
