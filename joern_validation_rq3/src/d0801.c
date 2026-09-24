void WebPagePrivate::rootLayerCommitTimerFired(Timer<WebPagePrivate>*)
{
    if (m_suspendRootLayerCommit)
        return;

#if DEBUG_AC_COMMIT
    BBLOG(Platform::LogLevelCritical, "%s", WTF_PRETTY_FUNCTION);
#endif

    m_backingStore->d->instrumentBeginFrame();

    requestLayoutIfNeeded();

    if (!compositorDrawsRootLayer()) {
        if (m_backingStore->d->isOpenGLCompositing() && m_backingStore->d->shouldDirectRenderingToWindow())
            setNeedsOneShotDrawingSynchronization();

        if (needsOneShotDrawingSynchronization()) {
#if DEBUG_AC_COMMIT
            BBLOG(Platform::LogLevelCritical, "%s: OneShotDrawingSynchronization code path!", WTF_PRETTY_FUNCTION);
#endif

            const IntRect windowRect = IntRect(IntPoint::zero(), viewportSize());
            m_backingStore->d->repaint(windowRect, true /*contentChanged*/, true /*immediate*/);
            return;
        }
    }

    commitRootLayerIfNeeded();
}
