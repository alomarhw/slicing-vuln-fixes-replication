void WebPage::notifyFullScreenVideoExited(bool done)
{
    UNUSED_PARAM(done);
#if ENABLE(VIDEO)
    if (d->m_webSettings->fullScreenVideoCapable()) {
        if (HTMLMediaElement* mediaElement = static_cast<HTMLMediaElement*>(d->m_fullscreenVideoNode.get()))
            mediaElement->exitFullscreen();
    } else {
#if ENABLE(FULLSCREEN_API)
        if (Element* element = static_cast<Element*>(d->m_fullscreenVideoNode.get()))
            element->document()->webkitCancelFullScreen();
#endif
    }
#endif
}
