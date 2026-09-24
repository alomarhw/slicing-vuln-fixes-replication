static int mov_switch_root(AVFormatContext *s, int64_t target)
{
    MOVContext *mov = s->priv_data;
    int i, j;
    int already_read = 0;

    if (avio_seek(s->pb, target, SEEK_SET) != target) {
        av_log(mov->fc, AV_LOG_ERROR, "root atom offset 0x%"PRIx64": partial file\n", target);
        return AVERROR_INVALIDDATA;
    }

    mov->next_root_atom = 0;

    for (i = 0; i < mov->fragment_index_count; i++) {
        MOVFragmentIndex *index = mov->fragment_index_data[i];
        int found = 0;
        for (j = 0; j < index->item_count; j++) {
            MOVFragmentIndexItem *item = &index->items[j];
            if (found) {
                mov->next_root_atom = item->moof_offset;
                break; // Advance to next index in outer loop
            } else if (item->moof_offset == target) {
                index->current_item = FFMIN(j, index->current_item);
                if (item->headers_read)
                    already_read = 1;
                item->headers_read = 1;
                found = 1;
            }
        }
        if (!found)
            index->current_item = 0;
    }

    if (already_read)
        return 0;

    mov->found_mdat = 0;

    if (mov_read_default(mov, s->pb, (MOVAtom){ AV_RL32("root"), INT64_MAX }) < 0 ||
        avio_feof(s->pb))
        return AVERROR_EOF;
    av_log(s, AV_LOG_TRACE, "read fragments, offset 0x%"PRIx64"\n", avio_tell(s->pb));

    return 1;
}
