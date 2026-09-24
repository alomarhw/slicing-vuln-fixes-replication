static int perf_trace_event_open(struct perf_event *p_event)
{
	struct ftrace_event_call *tp_event = p_event->tp_event;
	return tp_event->class->reg(tp_event, TRACE_REG_PERF_OPEN, p_event);
}
