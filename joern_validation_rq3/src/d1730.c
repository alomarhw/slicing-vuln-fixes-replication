e1000e_vm_state_change(void *opaque, int running, RunState state)
{
    E1000ECore *core = opaque;

    if (running) {
        trace_e1000e_vm_state_running();
        e1000e_intrmgr_resume(core);
        e1000e_autoneg_resume(core);
    } else {
        trace_e1000e_vm_state_stopped();
        e1000e_autoneg_pause(core);
        e1000e_intrmgr_pause(core);
    }
}
