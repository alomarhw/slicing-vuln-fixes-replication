ScriptPromise ImageBitmapFactories::CreateImageBitmapFromBlob(
    ScriptState* script_state,
    EventTarget& event_target,
    ImageBitmapSource* bitmap_source,
    base::Optional<IntRect> crop_rect,
    const ImageBitmapOptions* options) {
  Blob* blob = static_cast<Blob*>(bitmap_source);
  ImageBitmapLoader* loader = ImageBitmapFactories::ImageBitmapLoader::Create(
      From(event_target), crop_rect, options, script_state);
  ScriptPromise promise = loader->Promise();
  From(event_target).AddLoader(loader);
  loader->LoadBlobAsync(blob);
  return promise;
}
