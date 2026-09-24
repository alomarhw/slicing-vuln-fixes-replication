void WebPage::goToBackForwardEntry(BackForwardId id)
{
    HistoryItem* item = historyItemFromBackForwardId(id);
    ASSERT(item);
    d->m_page->goToItem(item, FrameLoadTypeIndexedBackForward);
}
