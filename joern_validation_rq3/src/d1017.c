void DevToolsDataSource::StartBundledDataRequest(
    const std::string& path,
    const content::URLDataSource::GotDataCallback& callback) {
  std::string filename = PathWithoutParams(path);
  base::StringPiece resource =
      content::DevToolsFrontendHost::GetFrontendResource(filename);

  DLOG_IF(WARNING, resource.empty())
      << "Unable to find dev tool resource: " << filename
      << ". If you compiled with debug_devtools=1, try running with "
         "--debug-devtools.";
  scoped_refptr<base::RefCountedStaticMemory> bytes(
      new base::RefCountedStaticMemory(resource.data(), resource.length()));
  callback.Run(bytes.get());
}
