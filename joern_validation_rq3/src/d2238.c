AXObject* AXObject::parentObjectIfExists() const {
  if (isDetached())
    return 0;

  if (m_parent)
    return m_parent;

  return computeParentIfExists();
}
