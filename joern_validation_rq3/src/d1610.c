bitposition(PG_FUNCTION_ARGS)
{
	VarBit	   *str = PG_GETARG_VARBIT_P(0);
	VarBit	   *substr = PG_GETARG_VARBIT_P(1);
	int			substr_length,
				str_length,
				i,
				is;
	bits8	   *s,				/* pointer into substring */
			   *p;				/* pointer into str */
	bits8		cmp,			/* shifted substring byte to compare */
				mask1,			/* mask for substring byte shifted right */
				mask2,			/* mask for substring byte shifted left */
				end_mask,		/* pad mask for last substring byte */
				str_mask;		/* pad mask for last string byte */
	bool		is_match;

	/* Get the substring length */
	substr_length = VARBITLEN(substr);
	str_length = VARBITLEN(str);

	/* String has zero length or substring longer than string, return 0 */
	if ((str_length == 0) || (substr_length > str_length))
		PG_RETURN_INT32(0);

	/* zero-length substring means return 1 */
	if (substr_length == 0)
		PG_RETURN_INT32(1);

	/* Initialise the padding masks */
	end_mask = BITMASK << VARBITPAD(substr);
	str_mask = BITMASK << VARBITPAD(str);
	for (i = 0; i < VARBITBYTES(str) - VARBITBYTES(substr) + 1; i++)
	{
		for (is = 0; is < BITS_PER_BYTE; is++)
		{
			is_match = true;
			p = VARBITS(str) + i;
			mask1 = BITMASK >> is;
			mask2 = ~mask1;
			for (s = VARBITS(substr);
				 is_match && s < VARBITEND(substr); s++)
			{
				cmp = *s >> is;
				if (s == VARBITEND(substr) - 1)
				{
					mask1 &= end_mask >> is;
					if (p == VARBITEND(str) - 1)
					{
						/* Check that there is enough of str left */
						if (mask1 & ~str_mask)
						{
							is_match = false;
							break;
						}
						mask1 &= str_mask;
					}
				}
				is_match = ((cmp ^ *p) & mask1) == 0;
				if (!is_match)
					break;
				/* Move on to the next byte */
				p++;
				if (p == VARBITEND(str))
				{
					mask2 = end_mask << (BITS_PER_BYTE - is);
					is_match = mask2 == 0;
#if 0
					elog(DEBUG4, "S. %d %d em=%2x sm=%2x r=%d",
						 i, is, end_mask, mask2, is_match);
#endif
					break;
				}
				cmp = *s << (BITS_PER_BYTE - is);
				if (s == VARBITEND(substr) - 1)
				{
					mask2 &= end_mask << (BITS_PER_BYTE - is);
					if (p == VARBITEND(str) - 1)
					{
						if (mask2 & ~str_mask)
						{
							is_match = false;
							break;
						}
						mask2 &= str_mask;
					}
				}
				is_match = ((cmp ^ *p) & mask2) == 0;
			}
			/* Have we found a match? */
			if (is_match)
				PG_RETURN_INT32(i * BITS_PER_BYTE + is + 1);
		}
	}
	PG_RETURN_INT32(0);
}
