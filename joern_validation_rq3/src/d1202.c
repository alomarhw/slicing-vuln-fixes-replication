onig_region_resize_clear(OnigRegion* region, int n)
{
  int r;
  
  r = onig_region_resize(region, n);
  if (r != 0) return r;
  onig_region_clear(region);
  return 0;
}
