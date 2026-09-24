static bool ExecuteDeleteForward(LocalFrame& frame,
                                 Event*,
                                 EditorCommandSource,
                                 const String&) {
  frame.GetEditor().DeleteWithDirection(
      DeleteDirection::kForward, TextGranularity::kCharacter, false, true);
  return true;
}
