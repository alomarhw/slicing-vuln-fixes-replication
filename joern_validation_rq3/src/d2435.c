bool ServiceWorkerDevToolsAgentHost::Close() {
  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::BindOnce(&TerminateServiceWorkerOnIO, context_weak_, version_id_));
  return true;
}
