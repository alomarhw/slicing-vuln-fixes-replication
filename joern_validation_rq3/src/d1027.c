base::DictionaryValue* GetTree(content::RenderViewHostImpl* rvh) {
  std::string frame_tree = rvh->frame_tree();
  EXPECT_FALSE(frame_tree.empty());
  base::Value* v = base::JSONReader::Read(frame_tree);
  base::DictionaryValue* tree = NULL;
  EXPECT_TRUE(v->IsType(base::Value::TYPE_DICTIONARY));
  EXPECT_TRUE(v->GetAsDictionary(&tree));
  return tree;
}
