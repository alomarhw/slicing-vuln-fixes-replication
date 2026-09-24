void AudioHandler::MakeConnection() {
  AtomicIncrement(&connection_ref_count_);

#if DEBUG_AUDIONODE_REFERENCES
  fprintf(stderr, "[%16p]: %16p: %2d: AudioHandler::ref   %3d [%3d]\n",
          Context(), this, GetNodeType(), connection_ref_count_,
          node_count_[GetNodeType()]);
#endif
  EnableOutputsIfNecessary();
}
