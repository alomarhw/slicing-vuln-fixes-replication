void RenderView::OnConnectTcpACK(
    int request_id,
    IPC::PlatformFileForTransit socket_for_transit,
    const PP_Flash_NetAddress& local_addr,
    const PP_Flash_NetAddress& remote_addr) {
  pepper_delegate_.OnConnectTcpACK(
      request_id,
      IPC::PlatformFileForTransitToPlatformFile(socket_for_transit),
      local_addr,
      remote_addr);
}
