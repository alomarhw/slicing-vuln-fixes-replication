  virtual Profile* GetOffTheRecordProfile() {
    if (IsOffTheRecord())
      return this;
    else
      return linked_profile_;
  }
