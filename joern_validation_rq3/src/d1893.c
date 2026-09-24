void WebPage::enableWebInspector()
{
    if (!d->m_inspectorClient)
        return;

    d->m_page->inspectorController()->connectFrontend(d->m_inspectorClient);
    d->m_page->settings()->setDeveloperExtrasEnabled(true);
    d->setPreventsScreenDimming(true);
}
