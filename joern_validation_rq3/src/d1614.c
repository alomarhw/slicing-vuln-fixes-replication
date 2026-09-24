void Document::decrementActiveParserCount()
{
    --m_activeParserCount;
    if (!frame())
        return;
#if ENABLE(THREADED_HTML_PARSER)
    loader()->checkLoadComplete();
#endif
    frame()->loader()->checkLoadComplete();
}
