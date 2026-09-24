int Layer::IndexOfChild(const Layer* reference) {
  for (size_t i = 0; i < children_.size(); ++i) {
    if (children_[i].get() == reference)
      return i;
  }
  return -1;
}
