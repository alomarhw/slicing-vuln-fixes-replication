static int snd_seq_open(struct inode *inode, struct file *file)
{
	int c, mode;			/* client id */
	struct snd_seq_client *client;
	struct snd_seq_user_client *user;
	int err;

	err = nonseekable_open(inode, file);
	if (err < 0)
		return err;

	if (mutex_lock_interruptible(&register_mutex))
		return -ERESTARTSYS;
	client = seq_create_client1(-1, SNDRV_SEQ_DEFAULT_EVENTS);
	if (client == NULL) {
		mutex_unlock(&register_mutex);
		return -ENOMEM;	/* failure code */
	}

	mode = snd_seq_file_flags(file);
	if (mode & SNDRV_SEQ_LFLG_INPUT)
		client->accept_input = 1;
	if (mode & SNDRV_SEQ_LFLG_OUTPUT)
		client->accept_output = 1;

	user = &client->data.user;
	user->fifo = NULL;
	user->fifo_pool_size = 0;

	if (mode & SNDRV_SEQ_LFLG_INPUT) {
		user->fifo_pool_size = SNDRV_SEQ_DEFAULT_CLIENT_EVENTS;
		user->fifo = snd_seq_fifo_new(user->fifo_pool_size);
		if (user->fifo == NULL) {
			seq_free_client1(client);
			kfree(client);
			mutex_unlock(&register_mutex);
			return -ENOMEM;
		}
	}

	usage_alloc(&client_usage, 1);
	client->type = USER_CLIENT;
	mutex_unlock(&register_mutex);

	c = client->number;
	file->private_data = client;

	/* fill client data */
	user->file = file;
	sprintf(client->name, "Client-%d", c);
	client->data.user.owner = get_pid(task_pid(current));

	/* make others aware this new client */
	snd_seq_system_client_ev_client_start(c);

	return 0;
}
