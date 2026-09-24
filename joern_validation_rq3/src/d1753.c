AudioMixerAlsa::~AudioMixerAlsa() {
  if (!thread_.get())
    return;
  DCHECK(MessageLoop::current() != thread_->message_loop());

  thread_->message_loop()->PostTask(
      FROM_HERE, base::Bind(&AudioMixerAlsa::Disconnect,
                            base::Unretained(this)));
  disconnected_event_.Wait();

  base::ThreadRestrictions::ScopedAllowIO allow_io_for_thread_join;
  thread_->Stop();
  thread_.reset();
}
