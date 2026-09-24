CURLcode Curl_upkeep(struct conncache *conn_cache,
                          void *data)
{
  /* Loop over every connection and make connection alive. */
  Curl_conncache_foreach(data,
                         conn_cache,
                         data,
                         conn_upkeep);
  return CURLE_OK;
}
