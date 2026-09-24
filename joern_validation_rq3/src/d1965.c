std::unique_ptr<AXValue> createRoleNameValue(AccessibilityRole role) {
  AtomicString roleName = AXObject::roleName(role);
  std::unique_ptr<AXValue> roleNameValue;
  if (!roleName.isNull()) {
    roleNameValue = createValue(roleName, AXValueTypeEnum::Role);
  } else {
    roleNameValue = createValue(AXObject::internalRoleName(role),
                                AXValueTypeEnum::InternalRole);
  }
  return roleNameValue;
}
