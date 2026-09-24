void InFlightBackendIO::DoomEntry(const std::string& key,
                                  const net::CompletionCallback& callback) {
  scoped_refptr<BackendIO> operation(new BackendIO(this, backend_, callback));
  operation->DoomEntry(key);
  PostOperation(FROM_HERE, operation.get());
}
