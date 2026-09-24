void GDataFileSystem::OnMoveEntryFromRootDirectoryCompleted(
    const FileOperationCallback& callback,
    const FilePath& file_path,
    const FilePath& dir_path,
    GDataErrorCode status,
    const GURL& document_url) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DCHECK(!callback.is_null());

  GDataFileError error = util::GDataToGDataFileError(status);
  if (error == GDATA_FILE_OK) {
    GDataEntry* entry = directory_service_->FindEntryByPathSync(file_path);
    if (entry) {
      DCHECK_EQ(directory_service_->root(), entry->parent());
      directory_service_->MoveEntryToDirectory(dir_path, entry,
          base::Bind(
              &GDataFileSystem::OnMoveEntryToDirectoryWithFileOperationCallback,
              ui_weak_ptr_,
              callback));
      return;
    } else {
      error = GDATA_FILE_ERROR_NOT_FOUND;
    }
  }

  callback.Run(error);
}
