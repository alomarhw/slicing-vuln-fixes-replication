AcpiDsResolveOperands (
    ACPI_WALK_STATE         *WalkState)
{
    UINT32                  i;
    ACPI_STATUS             Status = AE_OK;


    ACPI_FUNCTION_TRACE_PTR (DsResolveOperands, WalkState);


    /*
     * Attempt to resolve each of the valid operands
     * Method arguments are passed by reference, not by value. This means
     * that the actual objects are passed, not copies of the objects.
     */
    for (i = 0; i < WalkState->NumOperands; i++)
    {
        Status = AcpiExResolveToValue (&WalkState->Operands[i], WalkState);
        if (ACPI_FAILURE (Status))
        {
            break;
        }
    }

    return_ACPI_STATUS (Status);
}
