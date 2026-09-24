check_time (char const *file_name, struct timespec t)
{
  if (t.tv_sec < 0)
    WARNOPT (WARN_TIMESTAMP,
	     (0, 0, _("%s: implausibly old time stamp %s"),
	      file_name, tartime (t, true)));
  else if (timespec_cmp (volume_start_time, t) < 0)
    {
      struct timespec now;
      gettime (&now);
      if (timespec_cmp (now, t) < 0)
	{
	  char buf[TIMESPEC_STRSIZE_BOUND];
	  struct timespec diff;
	  diff.tv_sec = t.tv_sec - now.tv_sec;
	  diff.tv_nsec = t.tv_nsec - now.tv_nsec;
	  if (diff.tv_nsec < 0)
	    {
	      diff.tv_nsec += BILLION;
	      diff.tv_sec--;
	    }
	  WARNOPT (WARN_TIMESTAMP,
		   (0, 0, _("%s: time stamp %s is %s s in the future"),
		    file_name, tartime (t, true), code_timespec (diff, buf)));
	}
    }
}
