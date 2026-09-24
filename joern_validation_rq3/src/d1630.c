gsicc_init_profile_info(cmm_profile_t *profile)
{
    int k;

    /* Get the profile handle */
    profile->profile_handle =
        gsicc_get_profile_handle_buffer(profile->buffer,
                                        profile->buffer_size,
                                        profile->memory);
    if (profile->profile_handle == NULL)
        return -1;

    /* Compute the hash code of the profile. */
    gsicc_get_icc_buff_hash(profile->buffer, &(profile->hashcode),
                            profile->buffer_size);
    profile->hash_is_valid = true;
    profile->default_match = DEFAULT_NONE;
    profile->num_comps = gscms_get_input_channel_count(profile->profile_handle);
    profile->num_comps_out = gscms_get_output_channel_count(profile->profile_handle);
    profile->data_cs = gscms_get_profile_data_space(profile->profile_handle);

    /* Initialize the range to default values */
    for ( k = 0; k < profile->num_comps; k++) {
        profile->Range.ranges[k].rmin = 0.0;
        profile->Range.ranges[k].rmax = 1.0;
    }
    return 0;
}
