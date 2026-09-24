std::unique_ptr<HistogramBase> Histogram::PersistentCreate(
    const std::string& name,
    Sample minimum,
    Sample maximum,
    const BucketRanges* ranges,
    HistogramBase::AtomicCount* counts,
    HistogramBase::AtomicCount* logged_counts,
    uint32_t counts_size,
    HistogramSamples::Metadata* meta,
    HistogramSamples::Metadata* logged_meta) {
  return WrapUnique(new Histogram(name, minimum, maximum, ranges, counts,
                                  logged_counts, counts_size, meta,
                                  logged_meta));
}
