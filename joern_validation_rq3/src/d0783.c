bool DOMPatchSupport::removeChildAndMoveToNew(Digest* oldDigest, ExceptionCode& ec)
{
    RefPtr<Node> oldNode = oldDigest->m_node;
    if (!m_domEditor->removeChild(oldNode->parentNode(), oldNode.get(), ec))
        return false;

    UnusedNodesMap::iterator it = m_unusedNodesMap.find(oldDigest->m_sha1);
    if (it != m_unusedNodesMap.end()) {
        Digest* newDigest = it->second;
        Node* newNode = newDigest->m_node;
        if (!m_domEditor->replaceChild(newNode->parentNode(), oldNode, newNode, ec))
            return false;
        newDigest->m_node = oldNode.get();
        markNodeAsUsed(newDigest);
        return true;
    }

    for (size_t i = 0; i < oldDigest->m_children.size(); ++i) {
        if (!removeChildAndMoveToNew(oldDigest->m_children[i].get(), ec))
            return false;
    }
    return true;
}
