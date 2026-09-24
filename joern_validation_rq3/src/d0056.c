midi_synth_kill_note(int dev, int channel, int note, int velocity)
{
	int             orig_dev = synth_devs[dev]->midi_dev;
	int             msg, chn;

	if (note < 0 || note > 127)
		return 0;
	if (channel < 0 || channel > 15)
		return 0;
	if (velocity < 0)
		velocity = 0;
	if (velocity > 127)
		velocity = 127;

	leave_sysex(dev);

	msg = prev_out_status[orig_dev] & 0xf0;
	chn = prev_out_status[orig_dev] & 0x0f;

	if (chn == channel && ((msg == 0x90 && velocity == 64) || msg == 0x80))
	  {			/*
				 * Use running status
				 */
		  if (!prefix_cmd(orig_dev, note))
			  return 0;

		  midi_outc(orig_dev, note);

		  if (msg == 0x90)	/*
					 * Running status = Note on
					 */
			  midi_outc(orig_dev, 0);	/*
							   * Note on with velocity 0 == note
							   * off
							 */
		  else
			  midi_outc(orig_dev, velocity);
	} else
	  {
		  if (velocity == 64)
		    {
			    if (!prefix_cmd(orig_dev, 0x90 | (channel & 0x0f)))
				    return 0;
			    midi_outc(orig_dev, 0x90 | (channel & 0x0f));	/*
										 * Note on
										 */
			    midi_outc(orig_dev, note);
			    midi_outc(orig_dev, 0);	/*
							 * Zero G
							 */
		  } else
		    {
			    if (!prefix_cmd(orig_dev, 0x80 | (channel & 0x0f)))
				    return 0;
			    midi_outc(orig_dev, 0x80 | (channel & 0x0f));	/*
										 * Note off
										 */
			    midi_outc(orig_dev, note);
			    midi_outc(orig_dev, velocity);
		    }
	  }

	return 0;
}
