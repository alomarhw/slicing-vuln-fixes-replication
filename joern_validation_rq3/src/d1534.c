static long jas_iccpowi(int x, int n)
{
	long y;
	y = 1;
	while (--n >= 0)
		y *= x;
	return y;
 }
