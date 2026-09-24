  void Register(aura::Window* window) {
    window->AddObserver(this);
    auto policy = window->event_targeting_policy();
    window->SetEventTargetingPolicy(aura::EventTargetingPolicy::kNone);
    policy_map_.emplace(window, policy);
    for (auto* child : window->children())
      Register(child);
  }
