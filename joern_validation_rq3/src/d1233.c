PassRefPtr<Attr> Element::ensureAttr(const QualifiedName& name)
{
    AttrNodeList* attrNodeList = ensureAttrNodeListForElement(this);
    RefPtr<Attr> attrNode = findAttrNodeInList(attrNodeList, name);
    if (!attrNode) {
        attrNode = Attr::create(this, name);
        treeScope()->adoptIfNeeded(attrNode.get());
        attrNodeList->append(attrNode);
    }
    return attrNode.release();
}
