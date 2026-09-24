WriteFromUrlOperation::WriteFromUrlOperation(
    base::WeakPtr<OperationManager> manager,
    const ExtensionId& extension_id,
    net::URLRequestContextGetter* request_context,
    GURL url,
    const std::string& hash,
    const std::string& device_path,
    const base::FilePath& download_folder)
    : Operation(manager, extension_id, device_path, download_folder),
      request_context_(request_context),
      url_(url),
      hash_(hash),
      download_continuation_() {}
