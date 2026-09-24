acpi_os_physical_table_override(struct acpi_table_header *existing_table,
				acpi_physical_address *address,
				u32 *table_length)
{
	*table_length = 0;
	*address = 0;
	return AE_OK;
}
