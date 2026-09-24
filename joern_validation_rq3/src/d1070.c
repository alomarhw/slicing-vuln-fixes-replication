set_SUSP_CE(unsigned char *p, int location, int offset, int size)
{
	unsigned char *bp = p -1;
	/*  Extend the System Use Area
	 *   "CE" Format:
	 *               len  ver
	 *    +----+----+----+----+-----------+-----------+
	 *    | 'C'| 'E'| 1C | 01 | LOCATION1 | LOCATION2 |
	 *    +----+----+----+----+-----------+-----------+
	 *    0    1    2    3    4          12          20
	 *    +-----------+
	 *    | LOCATION3 |
	 *    +-----------+
	 *   20          28
	 *   LOCATION1 : Location of Continuation of System Use Area.
	 *   LOCATION2 : Offset to Start of Continuation.
	 *   LOCATION3 : Length of the Continuation.
	 */

	bp[1] = 'C';
	bp[2] = 'E';
	bp[3] = RR_CE_SIZE;	/* length	*/
	bp[4] = 1;		/* version	*/
	set_num_733(bp+5, location);
	set_num_733(bp+13, offset);
	set_num_733(bp+21, size);
	return (RR_CE_SIZE);
}
