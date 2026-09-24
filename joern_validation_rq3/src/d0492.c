vrrp_preempt_delay_handler(vector_t *strvec)
{
	vrrp_t *vrrp = LIST_TAIL_DATA(vrrp_data->vrrp);
	double preempt_delay;

	if (!read_double_strvec(strvec, 1, &preempt_delay, 0, TIMER_MAX_SEC, true)) {
		report_config_error(CONFIG_GENERAL_ERROR, "(%s) Preempt_delay not valid! must be between 0-%d", vrrp->iname, TIMER_MAX_SEC);
		vrrp->preempt_delay = 0;
	}
	else
		vrrp->preempt_delay = (unsigned long)(preempt_delay * TIMER_HZ);
}
