static uint8_t decode_scale_factors(ic_stream *ics, bitfile *ld)
{
    uint8_t g, sfb;
    int16_t t;
    int8_t noise_pcm_flag = 1;

    int16_t scale_factor = ics->global_gain;
    int16_t is_position = 0;
    int16_t noise_energy = ics->global_gain - 90;

    for (g = 0; g < ics->num_window_groups; g++)
    {
        for (sfb = 0; sfb < ics->max_sfb; sfb++)
        {
            switch (ics->sfb_cb[g][sfb])
            {
            case ZERO_HCB: /* zero book */
                ics->scale_factors[g][sfb] = 0;
#ifdef SF_PRINT
                printf("%d\n", ics->scale_factors[g][sfb]);
#endif
                break;
            case INTENSITY_HCB: /* intensity books */
            case INTENSITY_HCB2:

                /* decode intensity position */
                t = huffman_scale_factor(ld);
                is_position += (t - 60);
                ics->scale_factors[g][sfb] = is_position;
#ifdef SF_PRINT
                printf("%d\n", ics->scale_factors[g][sfb]);
#endif

                break;
            case NOISE_HCB: /* noise books */

#ifndef DRM
                /* decode noise energy */
                if (noise_pcm_flag)
                {
                    noise_pcm_flag = 0;
                    t = (int16_t)faad_getbits(ld, 9
                        DEBUGVAR(1,73,"scale_factor_data(): first noise")) - 256;
                } else {
                    t = huffman_scale_factor(ld);
                    t -= 60;
                }
                noise_energy += t;
                ics->scale_factors[g][sfb] = noise_energy;
#ifdef SF_PRINT
                printf("%d\n", ics->scale_factors[g][sfb]);
#endif
#else
                /* PNS not allowed in DRM */
                return 29;
#endif

                break;
            default: /* spectral books */

                /* ics->scale_factors[g][sfb] must be between 0 and 255 */

                ics->scale_factors[g][sfb] = 0;

                /* decode scale factor */
                t = huffman_scale_factor(ld);
                scale_factor += (t - 60);
                if (scale_factor < 0 || scale_factor > 255)
                    return 4;
                ics->scale_factors[g][sfb] = scale_factor;
#ifdef SF_PRINT
                printf("%d\n", ics->scale_factors[g][sfb]);
#endif

                break;
            }
        }
    }

    return 0;
}
