AtomicString FrameView::mediaType() const
{
    String overrideType;
    InspectorInstrumentation::applyEmulatedMedia(m_frame.get(), &overrideType);
    if (!overrideType.isNull())
        return AtomicString(overrideType);
    return m_mediaType;
}
