void WebPage::setDateTimeInput(const BlackBerry::Platform::String& value)
{
    if (d->m_page->defersLoading()) {
        d->m_deferredTasks.append(adoptPtr(new DeferredTaskSetDateTimeInput(d, value)));
        return;
    }
    DeferredTaskSetDateTimeInput::finishOrCancel(d);
    d->m_inputHandler->setInputValue(value);
}
