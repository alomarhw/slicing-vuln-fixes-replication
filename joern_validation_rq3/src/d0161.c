DownloadController::~DownloadController() {
  if (java_object_) {
    JNIEnv* env = base::android::AttachCurrentThread();
    env->DeleteWeakGlobalRef(java_object_->obj_);
    delete java_object_;
    base::android::CheckException(env);
  }
}
