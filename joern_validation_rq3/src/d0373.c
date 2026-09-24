DevToolsWindow::~DevToolsWindow() {
  content::DevToolsManager::GetInstance()->ClientHostClosing(
      frontend_host_.get());
  UpdateBrowserToolbar();

  DevToolsWindows* instances = &g_instances.Get();
  DevToolsWindows::iterator it(
      std::find(instances->begin(), instances->end(), this));
  DCHECK(it != instances->end());
  instances->erase(it);

  for (IndexingJobsMap::const_iterator jobs_it(indexing_jobs_.begin());
       jobs_it != indexing_jobs_.end(); ++jobs_it) {
    jobs_it->second->Stop();
  }
  indexing_jobs_.clear();
}
