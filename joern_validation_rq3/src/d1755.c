void RememberValidHandle(const ScriptableHandle* handle) {
  if (NULL == g_ValidHandles) {
    g_ValidHandles = new(std::nothrow) std::set<const ScriptableHandle*>;
    if (NULL == g_ValidHandles) {
      return;
    }
  }
   g_ValidHandles->insert(handle);
 }
