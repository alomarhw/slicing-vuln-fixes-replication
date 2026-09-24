static void update_flush(rdpContext* context)
{
	rdpUpdate* update = context->update;

	if (update->numberOrders > 0)
	{
		update->EndPaint(context);
		update->BeginPaint(context);
	}
}
