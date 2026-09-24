aspath_empty_get (void)
{
  struct aspath *aspath;

  aspath = aspath_new ();
  aspath_make_str_count (aspath);
  return aspath;
}
