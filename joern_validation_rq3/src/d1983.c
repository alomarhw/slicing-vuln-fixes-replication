static struct user_ta_elf *find_ta_elf(const TEE_UUID *uuid,
				       struct user_ta_ctx *utc)
{
	struct user_ta_elf *elf;

	TAILQ_FOREACH(elf, &utc->elfs, link)
		if (!memcmp(&elf->uuid, uuid, sizeof(*uuid)))
			return elf;
	return NULL;
}
