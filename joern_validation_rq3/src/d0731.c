int parse_number(char *arg, int *res)
{
	char *b;
	long number = strtol(arg, &b, 10);

	/* check for trailing junk after number */
	if(*b != '\0')
		return 0;

	/*
	 * check for strtol underflow or overflow in conversion.
	 * Note: strtol can validly return LONG_MIN and LONG_MAX
	 * if the user entered these values, but, additional code
	 * to distinguish this scenario is unnecessary, because for
	 * our purposes LONG_MIN and LONG_MAX are too large anyway
	 */
	if(number == LONG_MIN || number == LONG_MAX)
		return 0;

	/* reject negative numbers as invalid */
	if(number < 0)
		return 0;

	/* check if long result will overflow signed int */
	if(number > INT_MAX)
		return 0;

	*res = number;
	return 1;
}
