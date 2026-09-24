struct kvm_pit *kvm_create_pit(struct kvm *kvm, u32 flags)
{
	struct kvm_pit *pit;
	struct kvm_kpit_state *pit_state;
	int ret;

	pit = kzalloc(sizeof(struct kvm_pit), GFP_KERNEL);
	if (!pit)
		return NULL;

	pit->irq_source_id = kvm_request_irq_source_id(kvm);
	if (pit->irq_source_id < 0) {
		kfree(pit);
		return NULL;
	}

	mutex_init(&pit->pit_state.lock);
	mutex_lock(&pit->pit_state.lock);
	spin_lock_init(&pit->pit_state.inject_lock);

	kvm->arch.vpit = pit;
	pit->kvm = kvm;

	pit_state = &pit->pit_state;
	pit_state->pit = pit;
	hrtimer_init(&pit_state->pit_timer.timer,
		     CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
	pit_state->irq_ack_notifier.gsi = 0;
	pit_state->irq_ack_notifier.irq_acked = kvm_pit_ack_irq;
	kvm_register_irq_ack_notifier(kvm, &pit_state->irq_ack_notifier);
	pit_state->pit_timer.reinject = true;
	mutex_unlock(&pit->pit_state.lock);

	kvm_pit_reset(pit);

	pit->mask_notifier.func = pit_mask_notifer;
	kvm_register_irq_mask_notifier(kvm, 0, &pit->mask_notifier);

	kvm_iodevice_init(&pit->dev, &pit_dev_ops);
	ret = __kvm_io_bus_register_dev(&kvm->pio_bus, &pit->dev);
	if (ret < 0)
		goto fail;

	if (flags & KVM_PIT_SPEAKER_DUMMY) {
		kvm_iodevice_init(&pit->speaker_dev, &speaker_dev_ops);
		ret = __kvm_io_bus_register_dev(&kvm->pio_bus,
						&pit->speaker_dev);
		if (ret < 0)
			goto fail_unregister;
	}

	return pit;

fail_unregister:
	__kvm_io_bus_unregister_dev(&kvm->pio_bus, &pit->dev);

fail:
	if (pit->irq_source_id >= 0)
		kvm_free_irq_source_id(kvm, pit->irq_source_id);

	kfree(pit);
	return NULL;
}
