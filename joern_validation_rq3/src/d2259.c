  void OnMessageReceived(const IPC::Message& message) {
    IPC_BEGIN_MESSAGE_MAP(BrowserCompositorOutputSurfaceProxy, message)
      IPC_MESSAGE_HANDLER(GpuHostMsg_UpdateVSyncParameters,
                          OnUpdateVSyncParameters);
    IPC_END_MESSAGE_MAP()
  }
