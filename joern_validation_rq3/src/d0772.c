void MediaStreamManager::StopMediaStreamFromBrowser(const std::string& label) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  DeviceRequest* request = FindRequest(label);
  if (!request)
    return;

  if (request->device_stopped_cb) {
    for (const MediaStreamDevice& device : request->devices) {
      request->device_stopped_cb.Run(label, device);
    }
  }

  CancelRequest(label);
  IncrementDesktopCaptureCounter(DESKTOP_CAPTURE_NOTIFICATION_STOP);
}
