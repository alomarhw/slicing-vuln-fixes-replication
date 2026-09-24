static int snd_seq_ioctl_get_queue_timer(struct snd_seq_client *client,
					 void __user *arg)
{
	struct snd_seq_queue_timer timer;
	struct snd_seq_queue *queue;
	struct snd_seq_timer *tmr;

	if (copy_from_user(&timer, arg, sizeof(timer)))
		return -EFAULT;

	queue = queueptr(timer.queue);
	if (queue == NULL)
		return -EINVAL;

	if (mutex_lock_interruptible(&queue->timer_mutex)) {
		queuefree(queue);
		return -ERESTARTSYS;
	}
	tmr = queue->timer;
	memset(&timer, 0, sizeof(timer));
	timer.queue = queue->queue;

	timer.type = tmr->type;
	if (tmr->type == SNDRV_SEQ_TIMER_ALSA) {
		timer.u.alsa.id = tmr->alsa_id;
		timer.u.alsa.resolution = tmr->preferred_resolution;
	}
	mutex_unlock(&queue->timer_mutex);
	queuefree(queue);
	
	if (copy_to_user(arg, &timer, sizeof(timer)))
		return -EFAULT;
	return 0;
}
