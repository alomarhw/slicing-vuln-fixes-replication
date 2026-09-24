static void eval_map_stop(struct seq_file *m, void *v)
{
	mutex_unlock(&trace_eval_mutex);
}
