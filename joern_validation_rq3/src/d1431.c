HandleSignalsState DataPipeProducerDispatcher::GetHandleSignalsState() const {
  base::AutoLock lock(lock_);
  return GetHandleSignalsStateNoLock();
}
