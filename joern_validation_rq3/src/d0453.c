void show_q_histogram(const int counts[64], int max_buckets) {
 struct hist_bucket bucket[64];
 int buckets = 0;
 int total = 0;
 int scale;
 int i;

 for (i = 0; i < 64; i++) {
 if (counts[i]) {
      bucket[buckets].low = bucket[buckets].high = i;
      bucket[buckets].count = counts[i];
      buckets++;
      total += counts[i];
 }
 }

  fprintf(stderr, "\nQuantizer Selection:\n");
  scale = merge_hist_buckets(bucket, max_buckets, &buckets);
  show_histogram(bucket, buckets, total, scale);
}
