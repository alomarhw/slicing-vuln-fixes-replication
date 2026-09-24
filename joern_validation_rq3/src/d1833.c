int main(int argc, char **argv)
{
	char c;

	int i = unpack_bz2_stream(0, 1);
	if (i < 0)
		fprintf(stderr, "%s\n", bunzip_errors[-i]);
	else if (read(STDIN_FILENO, &c, 1))
		fprintf(stderr, "Trailing garbage ignored\n");
	return -i;
}
