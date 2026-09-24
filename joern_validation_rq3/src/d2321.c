static time_t gf_mktime_utc(struct tm *tm)
{
	static const time_t kTimeMax = ~(1L << (sizeof(time_t) * CHAR_BIT - 1));
	static const time_t kTimeMin = (1L << (sizeof(time_t) * CHAR_BIT - 1));
	time64_t result = timegm64(tm);
	if (result < kTimeMin || result > kTimeMax)
		return -1;
	return result;
}
