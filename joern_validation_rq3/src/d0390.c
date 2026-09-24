  static bool ConvertImageDataToBitmapValue(
      const v8::Arguments& args, Value** bitmap_value) {
    v8::Local<v8::Object> extension_args = args[1]->ToObject();
    v8::Local<v8::Object> details =
        extension_args->Get(v8::String::New("0"))->ToObject();
    v8::Local<v8::Object> image_data =
        details->Get(v8::String::New("imageData"))->ToObject();
    v8::Local<v8::Object> data =
        image_data->Get(v8::String::New("data"))->ToObject();
    int width = image_data->Get(v8::String::New("width"))->Int32Value();
    int height = image_data->Get(v8::String::New("height"))->Int32Value();

    int data_length = data->Get(v8::String::New("length"))->Int32Value();
    if (data_length != 4 * width * height) {
      NOTREACHED() << "Invalid argument to setIcon. Expecting ImageData.";
      return false;
    }

    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bitmap.allocPixels();
    bitmap.eraseARGB(0, 0, 0, 0);

    uint32_t* pixels = bitmap.getAddr32(0, 0);
    for (int t = 0; t < width*height; t++) {
      pixels[t] = SkPreMultiplyColor(
          ((data->Get(v8::Integer::New(4*t + 3))->Int32Value() & 0xFF) << 24) |
          ((data->Get(v8::Integer::New(4*t + 0))->Int32Value() & 0xFF) << 16) |
          ((data->Get(v8::Integer::New(4*t + 1))->Int32Value() & 0xFF) << 8) |
          ((data->Get(v8::Integer::New(4*t + 2))->Int32Value() & 0xFF) << 0));
    }

    IPC::Message bitmap_pickle;
    IPC::WriteParam(&bitmap_pickle, bitmap);
    *bitmap_value = BinaryValue::CreateWithCopiedBuffer(
        static_cast<const char*>(bitmap_pickle.data()), bitmap_pickle.size());

    return true;
  }
