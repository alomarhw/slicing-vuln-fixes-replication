static MXFDescriptor* mxf_resolve_multidescriptor(MXFContext *mxf, MXFDescriptor *descriptor, int track_id)
{
    MXFDescriptor *sub_descriptor = NULL;
    int i;

    if (!descriptor)
        return NULL;

    if (descriptor->type == MultipleDescriptor) {
        for (i = 0; i < descriptor->sub_descriptors_count; i++) {
            sub_descriptor = mxf_resolve_strong_ref(mxf, &descriptor->sub_descriptors_refs[i], Descriptor);

            if (!sub_descriptor) {
                av_log(mxf->fc, AV_LOG_ERROR, "could not resolve sub descriptor strong ref\n");
                continue;
            }
            if (sub_descriptor->linked_track_id == track_id) {
                return sub_descriptor;
            }
        }
    } else if (descriptor->type == Descriptor)
        return descriptor;

    return NULL;
}
