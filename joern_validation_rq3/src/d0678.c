void MetricsLog::RecordHistogramDelta(const std::string& histogram_name,
                                      const base::HistogramSamples& snapshot) {
  DCHECK(!closed_);
  EncodeHistogramDelta(histogram_name, snapshot, &uma_proto_);
}
