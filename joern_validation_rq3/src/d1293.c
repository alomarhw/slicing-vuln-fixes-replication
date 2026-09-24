  virtual void AnimationEnded(const ui::Animation* animation) {
    tabstrip_->FinishAnimation(this, layout_on_completion_);
  }
