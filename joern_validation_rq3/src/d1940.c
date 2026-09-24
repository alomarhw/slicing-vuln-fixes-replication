void DisplayItemList::copyCachedSubsequence(DisplayItems::iterator& currentIt, DisplayItems& updatedList)
{
    ASSERT(currentIt->isSubsequence());
    ASSERT(!currentIt->scope());
    DisplayItem::Id endSubsequenceId(currentIt->client(), DisplayItem::subsequenceTypeToEndSubsequenceType(currentIt->type()), 0);
    do {
        ASSERT(currentIt != m_currentDisplayItems.end());
        ASSERT(currentIt->isValid());
        updatedList.appendByMoving(*currentIt);
        ++currentIt;
    } while (!endSubsequenceId.matches(updatedList.last()));
}
