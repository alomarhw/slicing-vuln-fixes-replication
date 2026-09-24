GF_Err hdlr_Read(GF_Box *s, GF_BitStream *bs)
{
	GF_HandlerBox *ptr = (GF_HandlerBox *)s;

	ptr->reserved1 = gf_bs_read_u32(bs);
	ptr->handlerType = gf_bs_read_u32(bs);
	gf_bs_read_data(bs, (char*)ptr->reserved2, 12);

	ISOM_DECREASE_SIZE(ptr, 20);

	if (ptr->size) {
		size_t len;
		ptr->nameUTF8 = (char*)gf_malloc((u32) ptr->size);
		if (ptr->nameUTF8 == NULL) return GF_OUT_OF_MEM;
		gf_bs_read_data(bs, ptr->nameUTF8, (u32) ptr->size);
		/*safety check in case the string is not null-terminated*/
		if (ptr->nameUTF8[ptr->size-1]) {
			char *str = (char*)gf_malloc((u32) ptr->size + 1);
			memcpy(str, ptr->nameUTF8, (u32) ptr->size);
			str[ptr->size] = 0;
			gf_free(ptr->nameUTF8);
			ptr->nameUTF8 = str;
		}
		if (ptr->size > 1 && ptr->nameUTF8[0] == ptr->size-1) {
			len = strlen(ptr->nameUTF8 + 1);
			memmove(ptr->nameUTF8, ptr->nameUTF8+1, len );
			ptr->nameUTF8[len] = 0;
			ptr->store_counted_string = GF_TRUE;
		}
	}
	return GF_OK;
}
