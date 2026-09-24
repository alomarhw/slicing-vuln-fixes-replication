static void zrle_write_u24a(VncState *vs, uint32_t value)
{
    vnc_write(vs, (uint8_t *)&value, 3);
}
