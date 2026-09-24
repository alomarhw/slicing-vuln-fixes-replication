int32_t PepperDeviceEnumerationHostHelper::OnMonitorDeviceChange(
    HostMessageContext* /* context */,
    uint32_t callback_id) {
  monitor_.reset(new ScopedRequest(
      this,
      base::Bind(&PepperDeviceEnumerationHostHelper::OnNotifyDeviceChange,
                 base::Unretained(this),
                 callback_id)));

  return monitor_->requested() ? PP_OK : PP_ERROR_FAILED;
}
