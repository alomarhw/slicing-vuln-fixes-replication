void RendererSchedulerImpl::ApplyVirtualTimePolicy() {
  switch (main_thread_only().virtual_time_policy) {
    case VirtualTimePolicy::kAdvance:
      if (virtual_time_domain_) {
        virtual_time_domain_->SetMaxVirtualTimeTaskStarvationCount(
            main_thread_only().nested_runloop
                ? 0
                : main_thread_only().max_virtual_time_task_starvation_count);
        virtual_time_domain_->SetVirtualTimeFence(base::TimeTicks());
      }
      SetVirtualTimeStopped(false);
      break;
    case VirtualTimePolicy::kPause:
      if (virtual_time_domain_) {
        virtual_time_domain_->SetMaxVirtualTimeTaskStarvationCount(0);
        virtual_time_domain_->SetVirtualTimeFence(virtual_time_domain_->Now());
      }
      SetVirtualTimeStopped(true);
      break;
    case VirtualTimePolicy::kDeterministicLoading:
      if (virtual_time_domain_) {
        virtual_time_domain_->SetMaxVirtualTimeTaskStarvationCount(
            main_thread_only().nested_runloop
                ? 0
                : main_thread_only().max_virtual_time_task_starvation_count);
      }

      SetVirtualTimeStopped(main_thread_only().virtual_time_pause_count != 0 ||
                            main_thread_only().nested_runloop);
      break;
  }
}
