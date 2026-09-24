void DownloadItemDrag::BeginDrag(const DownloadItem* item, SkBitmap* icon) {
  new DownloadItemDrag(item, icon);
}
