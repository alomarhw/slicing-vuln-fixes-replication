asmlinkage long compat_sys_execve(const char __user * filename,
	const compat_uptr_t __user * argv,
	const compat_uptr_t __user * envp)
{
	struct filename *path = getname(filename);
	int error = PTR_ERR(path);
	if (!IS_ERR(path)) {
		error = compat_do_execve(path->name, argv, envp);
		putname(path);
	}
	return error;
}
