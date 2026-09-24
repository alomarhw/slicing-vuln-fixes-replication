void Smb4KGlobal::clearWorkgroupsList()
{
  mutex.lock();

  while (!p->workgroupsList.isEmpty())
  {
    delete p->workgroupsList.takeFirst();
  }

  mutex.unlock();
}
