void StartServerOnHandlerThread(
    base::WeakPtr<DevToolsHttpHandler> handler,
    std::unique_ptr<base::Thread> thread,
    std::unique_ptr<DevToolsSocketFactory> socket_factory,
    const base::FilePath& output_directory,
    const base::FilePath& debug_frontend_dir,
    const std::string& browser_guid,
    bool bundles_resources) {
  DCHECK(thread->task_runner()->BelongsToCurrentThread());
  std::unique_ptr<ServerWrapper> server_wrapper;
  std::unique_ptr<net::ServerSocket> server_socket =
      socket_factory->CreateForHttpServer();
  std::unique_ptr<net::IPEndPoint> ip_address(new net::IPEndPoint);
  if (server_socket) {
    server_wrapper.reset(new ServerWrapper(handler, std::move(server_socket),
                                           debug_frontend_dir,
                                           bundles_resources));
    if (server_wrapper->GetLocalAddress(ip_address.get()) != net::OK)
      ip_address.reset();
  } else {
    ip_address.reset();
  }

  if (ip_address) {
    std::string message = base::StringPrintf(
        "\nDevTools listening on ws://%s%s\n", ip_address->ToString().c_str(),
        browser_guid.c_str());
    fprintf(stderr, "%s", message.c_str());
    fflush(stderr);

    if (!output_directory.empty()) {
      base::FilePath path =
          output_directory.Append(kDevToolsActivePortFileName);
      std::string port_target_string = base::StringPrintf(
          "%d\n%s", ip_address->port(), browser_guid.c_str());
      if (base::WriteFile(path, port_target_string.c_str(),
                          static_cast<int>(port_target_string.length())) < 0) {
        LOG(ERROR) << "Error writing DevTools active port to file";
      }
    }
  } else {
    LOG(ERROR) << "Cannot start http server for devtools. Stop devtools.";
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::BindOnce(&ServerStartedOnUI, std::move(handler), thread.release(),
                     server_wrapper.release(), socket_factory.release(),
                     std::move(ip_address)));
}
