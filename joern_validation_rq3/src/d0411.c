static void free_subsystems(void)
{
	int i;

	for (i = 0; i < nr_subsystems; i++)
		free(subsystems[i]);
	free(subsystems);
	subsystems = NULL;
	nr_subsystems = 0;
}
