void DisplayItemList::invalidateUntracked(DisplayItemClient client)
{
    ASSERT(m_newDisplayItems.isEmpty());
    updateValidlyCachedClientsIfNeeded();
    m_validlyCachedClients.remove(client);
}
