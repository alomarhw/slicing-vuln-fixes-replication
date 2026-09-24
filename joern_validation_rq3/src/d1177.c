register_vrrp_scheduler_addresses(void)
{
	register_thread_address("vrrp_arp_thread", vrrp_arp_thread);
	register_thread_address("vrrp_dispatcher_init", vrrp_dispatcher_init);
	register_thread_address("vrrp_gratuitous_arp_thread", vrrp_gratuitous_arp_thread);
	register_thread_address("vrrp_lower_prio_gratuitous_arp_thread", vrrp_lower_prio_gratuitous_arp_thread);
	register_thread_address("vrrp_script_child_thread", vrrp_script_child_thread);
	register_thread_address("vrrp_script_thread", vrrp_script_thread);
	register_thread_address("vrrp_read_dispatcher_thread", vrrp_read_dispatcher_thread);
#ifdef _WITH_BFD_
	register_thread_address("vrrp_bfd_thread", vrrp_bfd_thread);
#endif
}
