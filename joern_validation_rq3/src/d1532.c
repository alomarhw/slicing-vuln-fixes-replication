static void try_auto_wep(struct airo_info *ai)
{
	if (auto_wep && !(ai->flags & FLAG_RADIO_DOWN)) {
		ai->expires = RUN_AT(3*HZ);
		wake_up_interruptible(&ai->thr_wait);
	}
}
