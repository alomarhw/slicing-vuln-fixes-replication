e1000e_set_imc(E1000ECore *core, int index, uint32_t val)
{
    trace_e1000e_irq_ims_clear_set_imc(val);
    e1000e_clear_ims_bits(core, val);
    e1000e_update_interrupt_state(core);
}
