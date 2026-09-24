void RunOpenDeviceCallback(const DeviceManager::OpenDeviceCallback& callback,
                           OpenDeviceError error) {
  callback.Run(error);
}
