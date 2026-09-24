static int amd_gpio_direction_input(struct gpio_chip *gc, unsigned offset)
{
	unsigned long flags;
	u32 pin_reg;
	struct amd_gpio *gpio_dev = gpiochip_get_data(gc);

	spin_lock_irqsave(&gpio_dev->lock, flags);
	pin_reg = readl(gpio_dev->base + offset * 4);
	/*
	 * Suppose BIOS or Bootloader sets specific debounce for the
	 * GPIO. if not, set debounce to be  2.75ms and remove glitch.
	*/
	if ((pin_reg & DB_TMR_OUT_MASK) == 0) {
		pin_reg |= 0xf;
		pin_reg |= BIT(DB_TMR_OUT_UNIT_OFF);
		pin_reg |= DB_TYPE_REMOVE_GLITCH << DB_CNTRL_OFF;
		pin_reg &= ~BIT(DB_TMR_LARGE_OFF);
	}

	pin_reg &= ~BIT(OUTPUT_ENABLE_OFF);
	writel(pin_reg, gpio_dev->base + offset * 4);
	spin_unlock_irqrestore(&gpio_dev->lock, flags);

	return 0;
}
