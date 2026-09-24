static void ftrace_ops_no_ops(unsigned long ip, unsigned long parent_ip)
{
	__ftrace_ops_list_func(ip, parent_ip, NULL, NULL);
}
