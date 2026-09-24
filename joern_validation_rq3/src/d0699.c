void FFmpegVideoDecoder::ReleaseFFmpegResources() {
  codec_context_.reset();
  av_frame_.reset();
}
