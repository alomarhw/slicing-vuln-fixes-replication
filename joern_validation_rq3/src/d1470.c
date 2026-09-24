aspath_aggregate (struct aspath *as1, struct aspath *as2)
{
  int i;
  int minlen;
  int match;
  int from;
  struct assegment *seg1 = as1->segments;
  struct assegment *seg2 = as2->segments;
  struct aspath *aspath = NULL;
  struct assegment *asset;
  struct assegment *prevseg = NULL;

  match = 0;
  minlen = 0;
  aspath = NULL;
  asset = NULL;

  /* First of all check common leading sequence. */
  while (seg1 && seg2)
    {      
      /* Check segment type. */
      if (seg1->type != seg2->type)
	break;

      /* Minimum segment length. */
      minlen = min (seg1->length, seg2->length);

      for (match = 0; match < minlen; match++)
	if (seg1->as[match] != seg2->as[match])
	  break;

      if (match)
	{
	  struct assegment *seg = assegment_new (seg1->type, 0);
	  
	  seg = assegment_append_asns (seg, seg1->as, match);

	  if (! aspath)
	    {
	      aspath = aspath_new ();
	      aspath->segments = seg;
	     }
	  else
	    prevseg->next = seg;
	  
	  prevseg = seg;
	}

      if (match != minlen || match != seg1->length 
	  || seg1->length != seg2->length)
	break;
      /* We are moving on to the next segment to reset match */
      else
        match = 0;
      
      seg1 = seg1->next;
      seg2 = seg2->next;
    }

  if (! aspath)
    aspath = aspath_new();

  /* Make as-set using rest of all information. */
  from = match;
  while (seg1)
    {
      for (i = from; i < seg1->length; i++)
	asset = aspath_aggregate_as_set_add (aspath, asset, seg1->as[i]);
      
      from = 0;
      seg1 = seg1->next;
    }

  from = match;
  while (seg2)
    {
      for (i = from; i < seg2->length; i++)
	asset = aspath_aggregate_as_set_add (aspath, asset, seg2->as[i]);

      from = 0;
      seg2 = seg2->next;
    }
  
  assegment_normalise (aspath->segments);
  aspath_str_update (aspath);
  return aspath;
}
