static void hardware_disable(void)
{
	if (vmm_exclusive) {
		vmclear_local_loaded_vmcss();
		kvm_cpu_vmxoff();
	}
	cr4_clear_bits(X86_CR4_VMXE);
}
