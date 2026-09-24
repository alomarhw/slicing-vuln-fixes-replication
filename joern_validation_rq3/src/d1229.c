  virtual void RunWork() {
    if (!file_util::DirectoryExists(file_path_.DirName())) {
      set_error_code(base::PLATFORM_FILE_ERROR_NOT_FOUND);
      return;
    }
    base::PlatformFileError error_code = base::PLATFORM_FILE_OK;
    file_handle_ = base::CreatePlatformFile(file_path_, file_flags_,
                                            &created_, &error_code);
    set_error_code(error_code);
  }
