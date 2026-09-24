bool current_in_userns(const struct user_namespace *target_ns)
{
	return in_userns(target_ns, current_user_ns());
}
