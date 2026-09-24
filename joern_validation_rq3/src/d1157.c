static void save_state_to_tss32(struct x86_emulate_ctxt *ctxt,
				struct tss_segment_32 *tss)
{
	/* CR3 and ldt selector are not saved intentionally */
	tss->eip = ctxt->_eip;
	tss->eflags = ctxt->eflags;
	tss->eax = reg_read(ctxt, VCPU_REGS_RAX);
	tss->ecx = reg_read(ctxt, VCPU_REGS_RCX);
	tss->edx = reg_read(ctxt, VCPU_REGS_RDX);
	tss->ebx = reg_read(ctxt, VCPU_REGS_RBX);
	tss->esp = reg_read(ctxt, VCPU_REGS_RSP);
	tss->ebp = reg_read(ctxt, VCPU_REGS_RBP);
	tss->esi = reg_read(ctxt, VCPU_REGS_RSI);
	tss->edi = reg_read(ctxt, VCPU_REGS_RDI);

	tss->es = get_segment_selector(ctxt, VCPU_SREG_ES);
	tss->cs = get_segment_selector(ctxt, VCPU_SREG_CS);
	tss->ss = get_segment_selector(ctxt, VCPU_SREG_SS);
	tss->ds = get_segment_selector(ctxt, VCPU_SREG_DS);
	tss->fs = get_segment_selector(ctxt, VCPU_SREG_FS);
	tss->gs = get_segment_selector(ctxt, VCPU_SREG_GS);
}
