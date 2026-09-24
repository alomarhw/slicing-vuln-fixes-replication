bool FFmpegVideoDecoder::FFmpegDecode(
    const scoped_refptr<DecoderBuffer>& buffer,
    scoped_refptr<VideoFrame>* video_frame) {
  DCHECK(video_frame);

  avcodec_get_frame_defaults(av_frame_.get());

  AVPacket packet;
  av_init_packet(&packet);
  if (buffer->end_of_stream()) {
    packet.data = NULL;
    packet.size = 0;
  } else {
    packet.data = const_cast<uint8*>(buffer->data());
    packet.size = buffer->data_size();

    codec_context_->reordered_opaque = buffer->timestamp().InMicroseconds();

    av_frame_->reordered_opaque = codec_context_->reordered_opaque;
  }

  int frame_decoded = 0;
  int result = avcodec_decode_video2(codec_context_.get(),
                                     av_frame_.get(),
                                     &frame_decoded,
                                     &packet);
  if (result < 0) {
    LOG(ERROR) << "Error decoding video: " << buffer->AsHumanReadableString();
    *video_frame = NULL;
    return false;
  }

  if (frame_decoded == 0) {
    *video_frame = NULL;
    return true;
  }

  if (!av_frame_->data[VideoFrame::kYPlane] ||
      !av_frame_->data[VideoFrame::kUPlane] ||
      !av_frame_->data[VideoFrame::kVPlane]) {
    LOG(ERROR) << "Video frame was produced yet has invalid frame data.";
    *video_frame = NULL;
    return false;
  }

  if (!av_frame_->opaque) {
    LOG(ERROR) << "VideoFrame object associated with frame data not set.";
    return false;
  }
  *video_frame = static_cast<VideoFrame*>(av_frame_->opaque);

  (*video_frame)->set_timestamp(
      base::TimeDelta::FromMicroseconds(av_frame_->reordered_opaque));

  return true;
}
