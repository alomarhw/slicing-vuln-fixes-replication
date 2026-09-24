void ResourceMessageFilter::OnOpenFileOnFileThread(const FilePath& path,
                                                   int mode,
                                                   IPC::Message* reply_msg) {
  DCHECK(ChromeThread::CurrentlyOn(ChromeThread::FILE));

  base::PlatformFile file_handle = base::CreatePlatformFile(
      path,
      (mode == 0) ? (base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ)
                  : (base::PLATFORM_FILE_CREATE_ALWAYS |
                        base::PLATFORM_FILE_WRITE),
      NULL);

  base::PlatformFile target_file_handle;
#if defined(OS_WIN)
  if (!DuplicateHandle(GetCurrentProcess(), file_handle,
                       handle(), &target_file_handle, 0, false,
                       DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS)) {
    target_file_handle = INVALID_HANDLE_VALUE;
  }
#else
  target_file_handle = file_handle;
#endif

  ViewHostMsg_OpenFile::WriteReplyParams(reply_msg, target_file_handle);

  ChromeThread::PostTask(
      ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(this, &ResourceMessageFilter::Send, reply_msg));
}
