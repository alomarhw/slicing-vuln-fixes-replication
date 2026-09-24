void regulator_unregister(struct regulator_dev *rdev)
{
	if (rdev == NULL)
		return;

	if (rdev->supply) {
		while (rdev->use_count--)
			regulator_disable(rdev->supply);
		regulator_put(rdev->supply);
	}
	mutex_lock(&regulator_list_mutex);
	debugfs_remove_recursive(rdev->debugfs);
	flush_work(&rdev->disable_work.work);
	WARN_ON(rdev->open_count);
	unset_regulator_supplies(rdev);
	list_del(&rdev->list);
	kfree(rdev->constraints);
	regulator_ena_gpio_free(rdev);
	of_node_put(rdev->dev.of_node);
	device_unregister(&rdev->dev);
	mutex_unlock(&regulator_list_mutex);
}
