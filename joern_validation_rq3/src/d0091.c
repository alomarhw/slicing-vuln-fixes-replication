fst_enable_intr(struct fst_card_info *card)
{
	if (card->family == FST_FAMILY_TXU) {
		outl(0x0f0c0900, card->pci_conf + INTCSR_9054);
	} else {
		outw(0x0543, card->pci_conf + INTCSR_9052);
	}
}
