void Document::fullScreenRendererDestroyed()
{
    m_fullScreenRenderer = 0;

    if (page())
        page()->chrome()->client()->fullScreenRendererChanged(0);
}
