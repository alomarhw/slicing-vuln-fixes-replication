static int vmx_get_vmx_msr(struct kvm_vcpu *vcpu, u32 msr_index, u64 *pdata)
{
	struct vcpu_vmx *vmx = to_vmx(vcpu);

	switch (msr_index) {
	case MSR_IA32_VMX_BASIC:
		*pdata = vmx->nested.nested_vmx_basic;
		break;
	case MSR_IA32_VMX_TRUE_PINBASED_CTLS:
	case MSR_IA32_VMX_PINBASED_CTLS:
		*pdata = vmx_control_msr(
			vmx->nested.nested_vmx_pinbased_ctls_low,
			vmx->nested.nested_vmx_pinbased_ctls_high);
		if (msr_index == MSR_IA32_VMX_PINBASED_CTLS)
			*pdata |= PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_TRUE_PROCBASED_CTLS:
	case MSR_IA32_VMX_PROCBASED_CTLS:
		*pdata = vmx_control_msr(
			vmx->nested.nested_vmx_procbased_ctls_low,
			vmx->nested.nested_vmx_procbased_ctls_high);
		if (msr_index == MSR_IA32_VMX_PROCBASED_CTLS)
			*pdata |= CPU_BASED_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_TRUE_EXIT_CTLS:
	case MSR_IA32_VMX_EXIT_CTLS:
		*pdata = vmx_control_msr(
			vmx->nested.nested_vmx_exit_ctls_low,
			vmx->nested.nested_vmx_exit_ctls_high);
		if (msr_index == MSR_IA32_VMX_EXIT_CTLS)
			*pdata |= VM_EXIT_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_TRUE_ENTRY_CTLS:
	case MSR_IA32_VMX_ENTRY_CTLS:
		*pdata = vmx_control_msr(
			vmx->nested.nested_vmx_entry_ctls_low,
			vmx->nested.nested_vmx_entry_ctls_high);
		if (msr_index == MSR_IA32_VMX_ENTRY_CTLS)
			*pdata |= VM_ENTRY_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_MISC:
		*pdata = vmx_control_msr(
			vmx->nested.nested_vmx_misc_low,
			vmx->nested.nested_vmx_misc_high);
		break;
	case MSR_IA32_VMX_CR0_FIXED0:
		*pdata = vmx->nested.nested_vmx_cr0_fixed0;
		break;
	case MSR_IA32_VMX_CR0_FIXED1:
		*pdata = vmx->nested.nested_vmx_cr0_fixed1;
		break;
	case MSR_IA32_VMX_CR4_FIXED0:
		*pdata = vmx->nested.nested_vmx_cr4_fixed0;
		break;
	case MSR_IA32_VMX_CR4_FIXED1:
		*pdata = vmx->nested.nested_vmx_cr4_fixed1;
		break;
	case MSR_IA32_VMX_VMCS_ENUM:
		*pdata = vmx->nested.nested_vmx_vmcs_enum;
		break;
	case MSR_IA32_VMX_PROCBASED_CTLS2:
		*pdata = vmx_control_msr(
			vmx->nested.nested_vmx_secondary_ctls_low,
			vmx->nested.nested_vmx_secondary_ctls_high);
		break;
	case MSR_IA32_VMX_EPT_VPID_CAP:
		*pdata = vmx->nested.nested_vmx_ept_caps |
			((u64)vmx->nested.nested_vmx_vpid_caps << 32);
		break;
	case MSR_IA32_VMX_VMFUNC:
		*pdata = vmx->nested.nested_vmx_vmfunc_controls;
		break;
	default:
		return 1;
	}

	return 0;
}
