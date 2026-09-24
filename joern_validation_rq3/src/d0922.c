void FileSystemOperation::DoOpenFile(const OpenFileCallback& callback,
                                     int file_flags) {
  FileSystemFileUtilProxy::CreateOrOpen(
      &operation_context_, src_util_, src_path_, file_flags,
      base::Bind(&FileSystemOperation::DidOpenFile,
                 base::Owned(this), callback));
}
