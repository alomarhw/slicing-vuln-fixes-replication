get_delayed_job_ct()
{
    tube t;
    size_t i;
    unsigned int count = 0;

    for (i = 0; i < tubes.used; i++) {
        t = tubes.items[i];
        count += t->delay.used;
    }
    return count;
}
