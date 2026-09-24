void GpuChannelHost::ForciblyCloseChannel() {
  Send(new GpuChannelMsg_CloseChannel());
  SetStateLost();
}
