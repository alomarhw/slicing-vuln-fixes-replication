void Document::updateRangesAfterNodeMovedToAnotherDocument(const Node& node)
{
    ASSERT(node.document() != this);
    if (m_ranges.isEmpty())
        return;

    AttachedRangeSet ranges = m_ranges;
    for (Range* range : ranges)
        range->updateOwnerDocumentIfNeeded();
}
