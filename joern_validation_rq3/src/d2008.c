V0CustomElementMicrotaskRunQueue* Document::CustomElementMicrotaskRunQueue() {
  if (!custom_element_microtask_run_queue_)
    custom_element_microtask_run_queue_ =
        V0CustomElementMicrotaskRunQueue::Create();
  return custom_element_microtask_run_queue_.Get();
}
