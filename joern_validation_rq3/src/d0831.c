int WebPage::backForwardListLength() const
{
    return d->m_page->getHistoryLength();
}
