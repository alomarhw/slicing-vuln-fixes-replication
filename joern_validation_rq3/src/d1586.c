int kvm_is_error_hva(unsigned long addr)
{
	return addr == bad_hva();
}
