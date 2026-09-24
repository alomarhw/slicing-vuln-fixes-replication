static int acpi_pcihp_get_bsel(PCIBus *bus)
{
    Error *local_err = NULL;
    int64_t bsel = object_property_get_int(OBJECT(bus), ACPI_PCIHP_PROP_BSEL,
                                           &local_err);

    if (local_err || bsel < 0 || bsel >= ACPI_PCIHP_MAX_HOTPLUG_BUS) {
        if (local_err) {
            error_free(local_err);
        }
        return -1;
    } else {
        return bsel;
    }
}
