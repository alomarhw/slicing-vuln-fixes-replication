void __init ip_static_sysctl_init(void)
{
	register_sysctl_paths(ipv4_path, ipv4_skeleton);
}
