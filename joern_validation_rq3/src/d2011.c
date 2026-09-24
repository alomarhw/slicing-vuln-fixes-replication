void VideoCaptureManager::ConnectClient(
    media::VideoCaptureSessionId session_id,
    const media::VideoCaptureParams& params,
    VideoCaptureControllerID client_id,
    VideoCaptureControllerEventHandler* client_handler,
    const DoneCB& done_cb) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  {
    std::ostringstream string_stream;
    string_stream << "ConnectClient: session_id = " << session_id
                  << ", request: "
                  << media::VideoCaptureFormat::ToString(
                         params.requested_format);
    EmitLogMessage(string_stream.str(), 1);
  }

  VideoCaptureController* controller =
      GetOrCreateController(session_id, params);
  if (!controller) {
    done_cb.Run(base::WeakPtr<VideoCaptureController>());
    return;
  }

  LogVideoCaptureEvent(VIDEO_CAPTURE_START_CAPTURE);

  if (!controller->HasActiveClient() && !controller->HasPausedClient()) {
    std::ostringstream string_stream;
    string_stream
        << "VideoCaptureManager queueing device start for device_id = "
        << controller->device_id();
    EmitLogMessage(string_stream.str(), 1);
    QueueStartDevice(session_id, controller, params);
  }
  done_cb.Run(controller->GetWeakPtrForIOThread());
  controller->AddClient(client_id, client_handler, session_id, params);
}
