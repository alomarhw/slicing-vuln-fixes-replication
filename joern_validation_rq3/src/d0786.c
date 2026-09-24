  void OnLostSharedContext() {
    scoped_ptr<WebGraphicsContext3DCommandBufferImpl> old_shared_context(
        shared_context_.release());
    scoped_ptr<GLHelper> old_helper(gl_helper_.release());

    FOR_EACH_OBSERVER(ImageTransportFactoryObserver,
        observer_list_,
        OnLostResources());
  }
