void WallpaperManagerBase::UpdateWallpaper(bool clear_cache) {
  for (auto& observer : observers_)
    observer.OnUpdateWallpaperForTesting();
  if (clear_cache)
    wallpaper_cache_.clear();
  SetUserWallpaperNow(last_selected_user_);
}
