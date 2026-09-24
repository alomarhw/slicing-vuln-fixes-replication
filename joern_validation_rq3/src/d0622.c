void PreconnectManager::StartPreresolveHosts(
    const std::vector<std::string>& hostnames) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  for (auto it = hostnames.rbegin(); it != hostnames.rend(); ++it) {
    PreresolveJobId job_id =
        preresolve_jobs_.Add(std::make_unique<PreresolveJob>(
            GURL("http://" + *it), 0, kAllowCredentialsOnPreconnectByDefault,
            net::NetworkIsolationKey(), nullptr));
    queued_jobs_.push_front(job_id);
  }

  TryToLaunchPreresolveJobs();
}
