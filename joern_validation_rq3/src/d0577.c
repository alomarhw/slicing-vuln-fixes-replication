  void ProcessScrollNoBoundsCheck(ui::EventType type,
                                  const gfx::Vector2dF& delta) {
    ProcessScrollInternal(type, delta, false);
  }
