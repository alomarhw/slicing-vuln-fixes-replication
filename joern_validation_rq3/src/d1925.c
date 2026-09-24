void Range::nodeWillBeRemoved(Node* node)
{
    ASSERT(node);
    ASSERT(node->document() == m_ownerDocument);
    ASSERT(node != m_ownerDocument);
    ASSERT(node->parentNode());
    boundaryNodeWillBeRemoved(m_start, node);
    boundaryNodeWillBeRemoved(m_end, node);
}
