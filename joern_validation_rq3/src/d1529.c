WebContentsAndroid::~WebContentsAndroid() {
  Java_WebContentsImpl_clearNativePtr(AttachCurrentThread(), obj_.obj());
}
