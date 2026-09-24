static Vector<AtomicString>* createRoleNameVector() {
  Vector<AtomicString>* roleNameVector = new Vector<AtomicString>(NumRoles);
  for (int i = 0; i < NumRoles; i++)
    (*roleNameVector)[i] = nullAtom;

  for (size_t i = 0; i < WTF_ARRAY_LENGTH(roles); ++i)
    (*roleNameVector)[roles[i].webcoreRole] = AtomicString(roles[i].ariaRole);

  for (size_t i = 0; i < WTF_ARRAY_LENGTH(reverseRoles); ++i)
    (*roleNameVector)[reverseRoles[i].webcoreRole] =
        AtomicString(reverseRoles[i].ariaRole);

  return roleNameVector;
}
