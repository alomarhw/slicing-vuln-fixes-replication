bool AudioMixerAlsa::IsMuted() {
  base::AutoLock lock(lock_);
  return is_muted_;
}
