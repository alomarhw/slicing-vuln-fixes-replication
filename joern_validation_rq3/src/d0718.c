void SavePackage::FinalizeDownloadEntry() {
  DCHECK(download_);
  DCHECK(download_manager_);

  NotificationService::current()->Notify(
      NOTIFICATION_SAVE_PACKAGE_SUCCESSFULLY_FINISHED,
      Source<DownloadManager>(download_manager_),
      Details<DownloadItem>(download_));
  StopObservation();
}
