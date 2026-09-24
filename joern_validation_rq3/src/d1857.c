void DatabaseMessageFilter::OnDatabaseGetUsageAndQuota(
    IPC::Message* reply_msg,
    quota::QuotaStatusCode status,
    int64 usage,
    int64 quota) {
  int64 available = 0;
  if ((status == quota::kQuotaStatusOk) && (usage < quota))
    available = quota - usage;
  DatabaseHostMsg_GetSpaceAvailable::WriteReplyParams(reply_msg, available);
  Send(reply_msg);
}
