    FromMojom(media::mojom::DecodeError error,
              media::JpegDecodeAccelerator::Error* out) {
  switch (error) {
    case media::mojom::DecodeError::NO_ERRORS:
      *out = media::JpegDecodeAccelerator::Error::NO_ERRORS;
      return true;
    case media::mojom::DecodeError::INVALID_ARGUMENT:
      *out = media::JpegDecodeAccelerator::Error::INVALID_ARGUMENT;
      return true;
    case media::mojom::DecodeError::UNREADABLE_INPUT:
      *out = media::JpegDecodeAccelerator::Error::UNREADABLE_INPUT;
      return true;
    case media::mojom::DecodeError::PARSE_JPEG_FAILED:
      *out = media::JpegDecodeAccelerator::Error::PARSE_JPEG_FAILED;
      return true;
    case media::mojom::DecodeError::UNSUPPORTED_JPEG:
      *out = media::JpegDecodeAccelerator::Error::UNSUPPORTED_JPEG;
      return true;
    case media::mojom::DecodeError::PLATFORM_FAILURE:
      *out = media::JpegDecodeAccelerator::Error::PLATFORM_FAILURE;
      return true;
  }
  NOTREACHED();
  return false;
}
