void AwContents::OnPermissionRequest(
    base::android::ScopedJavaLocalRef<jobject> j_request,
    AwPermissionRequest* request) {
  DCHECK(!j_request.is_null());
  DCHECK(request);

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> j_ref = java_ref_.get(env);
  if (j_ref.is_null()) {
    permission_request_handler_->CancelRequest(request->GetOrigin(),
                                               request->GetResources());
    return;
  }

  Java_AwContents_onPermissionRequest(env, j_ref.obj(), j_request.obj());
}
