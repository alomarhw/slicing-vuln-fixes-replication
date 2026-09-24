void UsageTracker::UpdateUsageCache(
    QuotaClient::ID client_id, const GURL& origin, int64 delta) {
  ClientUsageTracker* client_tracker = GetClientTracker(client_id);
  DCHECK(client_tracker);
   client_tracker->UpdateUsageCache(origin, delta);
 }
