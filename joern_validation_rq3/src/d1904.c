bool CompareTrees(base::DictionaryValue* first, base::DictionaryValue* second) {
  string16 name1;
  string16 name2;
  if (!first->GetString(content::kFrameTreeNodeNameKey, &name1) ||
      !second->GetString(content::kFrameTreeNodeNameKey, &name2))
    return false;
  if (name1 != name2)
    return false;

  int id1 = 0;
  int id2 = 0;
  if (!first->GetInteger(content::kFrameTreeNodeIdKey, &id1) ||
      !second->GetInteger(content::kFrameTreeNodeIdKey, &id2)) {
    return false;
  }
  if (id1 != id2)
    return false;

  ListValue* subtree1 = NULL;
  ListValue* subtree2 = NULL;
  bool result1 = first->GetList(content::kFrameTreeNodeSubtreeKey, &subtree1);
  bool result2 = second->GetList(content::kFrameTreeNodeSubtreeKey, &subtree2);
  if (!result1 && !result2)
    return true;
  if (!result1 || !result2)
    return false;

  if (subtree1->GetSize() != subtree2->GetSize())
    return false;

  base::DictionaryValue* child1 = NULL;
  base::DictionaryValue* child2 = NULL;
  for (size_t i = 0; i < subtree1->GetSize(); ++i) {
    if (!subtree1->GetDictionary(i, &child1) ||
        !subtree2->GetDictionary(i, &child2)) {
      return false;
    }
    if (!CompareTrees(child1, child2))
      return false;
  }

  return true;
}
