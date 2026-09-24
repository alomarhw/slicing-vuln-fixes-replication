bool GLES2DecoderImpl::Initialize(
    const scoped_refptr<gfx::GLSurface>& surface,
    const scoped_refptr<gfx::GLContext>& context,
    bool offscreen,
    const gfx::Size& size,
    const DisallowedFeatures& disallowed_features,
    const std::vector<int32>& attribs) {
  TRACE_EVENT0("gpu", "GLES2DecoderImpl::Initialize");
  DCHECK(context->IsCurrent(surface.get()));
  DCHECK(!context_.get());

  set_initialized();
  gpu_tracer_ = GPUTracer::Create(this);
  gpu_state_tracer_ = GPUStateTracer::Create(&state_);
  gpu_trace_level_ = 2;
  gpu_trace_commands_ = false;

  if (CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kEnableGPUDebugging)) {
    set_debug(true);
  }

  if (CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kEnableGPUCommandLogging)) {
    set_log_commands(true);
  }

  compile_shader_always_succeeds_ = CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kCompileShaderAlwaysSucceeds);


  context_ = context;
  surface_ = surface;

  ContextCreationAttribHelper attrib_parser;
  if (!attrib_parser.Parse(attribs))
    return false;

  lose_context_when_out_of_memory_ =
      attrib_parser.lose_context_when_out_of_memory_;

  if (attrib_parser.fail_if_major_perf_caveat_ &&
      feature_info_->feature_flags().is_swiftshader) {
    group_ = NULL;  // Must not destroy ContextGroup if it is not initialized.
    Destroy(true);
    return false;
  }

  if (!group_->Initialize(this, disallowed_features)) {
    LOG(ERROR) << "GpuScheduler::InitializeCommon failed because group "
               << "failed to initialize.";
    group_ = NULL;  // Must not destroy ContextGroup if it is not initialized.
    Destroy(true);
    return false;
  }
  CHECK_GL_ERROR();

  disallowed_features_ = disallowed_features;

  state_.attrib_values.resize(group_->max_vertex_attribs());
  vertex_array_manager_.reset(new VertexArrayManager());

  GLuint default_vertex_attrib_service_id = 0;
  if (features().native_vertex_array_object) {
    glGenVertexArraysOES(1, &default_vertex_attrib_service_id);
    glBindVertexArrayOES(default_vertex_attrib_service_id);
  }

  state_.default_vertex_attrib_manager =
      CreateVertexAttribManager(0, default_vertex_attrib_service_id, false);

  state_.default_vertex_attrib_manager->Initialize(
      group_->max_vertex_attribs(),
      feature_info_->workarounds().init_vertex_attributes);

  DoBindVertexArrayOES(0);

  query_manager_.reset(new QueryManager(this, feature_info_.get()));

  util_.set_num_compressed_texture_formats(
      validators_->compressed_texture_format.GetValues().size());

  if (gfx::GetGLImplementation() != gfx::kGLImplementationEGLGLES2) {
    glEnableVertexAttribArray(0);
  }
  glGenBuffersARB(1, &attrib_0_buffer_id_);
  glBindBuffer(GL_ARRAY_BUFFER, attrib_0_buffer_id_);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glGenBuffersARB(1, &fixed_attrib_buffer_id_);

  state_.texture_units.resize(group_->max_texture_units());
  for (uint32 tt = 0; tt < state_.texture_units.size(); ++tt) {
    glActiveTexture(GL_TEXTURE0 + tt);
    TextureRef* ref;
    if (features().oes_egl_image_external) {
      ref = texture_manager()->GetDefaultTextureInfo(
          GL_TEXTURE_EXTERNAL_OES);
      state_.texture_units[tt].bound_texture_external_oes = ref;
      glBindTexture(GL_TEXTURE_EXTERNAL_OES, ref ? ref->service_id() : 0);
    }
    if (features().arb_texture_rectangle) {
      ref = texture_manager()->GetDefaultTextureInfo(
          GL_TEXTURE_RECTANGLE_ARB);
      state_.texture_units[tt].bound_texture_rectangle_arb = ref;
      glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ref ? ref->service_id() : 0);
    }
    ref = texture_manager()->GetDefaultTextureInfo(GL_TEXTURE_CUBE_MAP);
    state_.texture_units[tt].bound_texture_cube_map = ref;
    glBindTexture(GL_TEXTURE_CUBE_MAP, ref ? ref->service_id() : 0);
    ref = texture_manager()->GetDefaultTextureInfo(GL_TEXTURE_2D);
    state_.texture_units[tt].bound_texture_2d = ref;
    glBindTexture(GL_TEXTURE_2D, ref ? ref->service_id() : 0);
  }
  glActiveTexture(GL_TEXTURE0);
  CHECK_GL_ERROR();

  if (offscreen) {
    if (attrib_parser.samples_ > 0 && attrib_parser.sample_buffers_ > 0 &&
        features().chromium_framebuffer_multisample) {
      GLint max_sample_count = 1;
      glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_sample_count);
      offscreen_target_samples_ = std::min(attrib_parser.samples_,
                                           max_sample_count);
    } else {
      offscreen_target_samples_ = 1;
    }
    offscreen_target_buffer_preserved_ = attrib_parser.buffer_preserved_;

    if (gfx::GetGLImplementation() == gfx::kGLImplementationEGLGLES2) {
      const bool rgb8_supported =
          context_->HasExtension("GL_OES_rgb8_rgba8");
      if (rgb8_supported && offscreen_target_samples_ > 1) {
        offscreen_target_color_format_ = attrib_parser.alpha_size_ > 0 ?
            GL_RGBA8 : GL_RGB8;
      } else {
        offscreen_target_samples_ = 1;
        offscreen_target_color_format_ = attrib_parser.alpha_size_ > 0 ?
            GL_RGBA : GL_RGB;
      }

      const bool depth24_stencil8_supported =
          feature_info_->feature_flags().packed_depth24_stencil8;
      VLOG(1) << "GL_OES_packed_depth_stencil "
              << (depth24_stencil8_supported ? "" : "not ") << "supported.";
      if ((attrib_parser.depth_size_ > 0 || attrib_parser.stencil_size_ > 0) &&
          depth24_stencil8_supported) {
        offscreen_target_depth_format_ = GL_DEPTH24_STENCIL8;
        offscreen_target_stencil_format_ = 0;
      } else {
        offscreen_target_depth_format_ = attrib_parser.depth_size_ > 0 ?
            GL_DEPTH_COMPONENT16 : 0;
        offscreen_target_stencil_format_ = attrib_parser.stencil_size_ > 0 ?
            GL_STENCIL_INDEX8 : 0;
      }
    } else {
      offscreen_target_color_format_ = attrib_parser.alpha_size_ > 0 ?
          GL_RGBA : GL_RGB;

      const bool depth24_stencil8_supported =
          feature_info_->feature_flags().packed_depth24_stencil8;
      VLOG(1) << "GL_EXT_packed_depth_stencil "
              << (depth24_stencil8_supported ? "" : "not ") << "supported.";

      if ((attrib_parser.depth_size_ > 0 || attrib_parser.stencil_size_ > 0) &&
          depth24_stencil8_supported) {
        offscreen_target_depth_format_ = GL_DEPTH24_STENCIL8;
        offscreen_target_stencil_format_ = 0;
      } else {
        offscreen_target_depth_format_ = attrib_parser.depth_size_ > 0 ?
            GL_DEPTH_COMPONENT : 0;
        offscreen_target_stencil_format_ = attrib_parser.stencil_size_ > 0 ?
            GL_STENCIL_INDEX : 0;
      }
    }

    offscreen_saved_color_format_ = attrib_parser.alpha_size_ > 0 ?
        GL_RGBA : GL_RGB;

    offscreen_target_frame_buffer_.reset(new BackFramebuffer(this));
    offscreen_target_frame_buffer_->Create();
    if (IsOffscreenBufferMultisampled()) {
      offscreen_target_color_render_buffer_.reset(new BackRenderbuffer(
          renderbuffer_manager(), memory_tracker(), &state_));
      offscreen_target_color_render_buffer_->Create();
    } else {
      offscreen_target_color_texture_.reset(new BackTexture(
          memory_tracker(), &state_));
      offscreen_target_color_texture_->Create();
    }
    offscreen_target_depth_render_buffer_.reset(new BackRenderbuffer(
        renderbuffer_manager(), memory_tracker(), &state_));
    offscreen_target_depth_render_buffer_->Create();
    offscreen_target_stencil_render_buffer_.reset(new BackRenderbuffer(
        renderbuffer_manager(), memory_tracker(), &state_));
    offscreen_target_stencil_render_buffer_->Create();

    offscreen_saved_frame_buffer_.reset(new BackFramebuffer(this));
    offscreen_saved_frame_buffer_->Create();
    offscreen_saved_color_texture_.reset(new BackTexture(
        memory_tracker(), &state_));
    offscreen_saved_color_texture_->Create();

    if (!ResizeOffscreenFrameBuffer(size)) {
      LOG(ERROR) << "Could not allocate offscreen buffer storage.";
      Destroy(true);
      return false;
    }

    DCHECK(offscreen_saved_color_format_);
    offscreen_saved_color_texture_->AllocateStorage(
        gfx::Size(1, 1), offscreen_saved_color_format_, true);

    offscreen_saved_frame_buffer_->AttachRenderTexture(
        offscreen_saved_color_texture_.get());
    if (offscreen_saved_frame_buffer_->CheckStatus() !=
        GL_FRAMEBUFFER_COMPLETE) {
      LOG(ERROR) << "Offscreen saved FBO was incomplete.";
      Destroy(true);
      return false;
    }

    DoBindFramebuffer(GL_FRAMEBUFFER, 0);
  } else {
    glBindFramebufferEXT(GL_FRAMEBUFFER, GetBackbufferServiceId());

    GLint v = 0;
    glGetIntegerv(GL_ALPHA_BITS, &v);
    back_buffer_color_format_ =
        (attrib_parser.alpha_size_ != 0 && v > 0) ? GL_RGBA : GL_RGB;
    glGetIntegerv(GL_DEPTH_BITS, &v);
    back_buffer_has_depth_ = attrib_parser.depth_size_ != 0 && v > 0;
    glGetIntegerv(GL_STENCIL_BITS, &v);
    back_buffer_has_stencil_ = attrib_parser.stencil_size_ != 0 && v > 0;
  }

  if (gfx::GetGLImplementation() != gfx::kGLImplementationEGLGLES2) {
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
  }

  has_robustness_extension_ =
      context->HasExtension("GL_ARB_robustness") ||
      context->HasExtension("GL_EXT_robustness");

  if (!InitializeShaderTranslator()) {
    return false;
  }

  state_.viewport_width = size.width();
  state_.viewport_height = size.height();

  GLint viewport_params[4] = { 0 };
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewport_params);
  viewport_max_width_ = viewport_params[0];
  viewport_max_height_ = viewport_params[1];

  state_.scissor_width = state_.viewport_width;
  state_.scissor_height = state_.viewport_height;

  state_.InitCapabilities(NULL);
  state_.InitState(NULL);
  glActiveTexture(GL_TEXTURE0 + state_.active_texture_unit);

  DoBindBuffer(GL_ARRAY_BUFFER, 0);
  DoBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  DoBindFramebuffer(GL_FRAMEBUFFER, 0);
  DoBindRenderbuffer(GL_RENDERBUFFER, 0);

  bool call_gl_clear = true;
#if defined(OS_ANDROID)
  call_gl_clear = surface_->GetHandle();
#endif
  if (call_gl_clear) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }

  supports_post_sub_buffer_ = surface->SupportsPostSubBuffer();
  if (feature_info_->workarounds()
          .disable_post_sub_buffers_for_onscreen_surfaces &&
      !surface->IsOffscreen())
    supports_post_sub_buffer_ = false;

  if (feature_info_->workarounds().reverse_point_sprite_coord_origin) {
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
  }

  if (feature_info_->workarounds().unbind_fbo_on_context_switch) {
    context_->SetUnbindFboOnMakeCurrent();
  }

  if (feature_info_->workarounds().release_image_after_use) {
    image_manager()->SetReleaseAfterUse();
  }

  if (!offscreen)
    context_->SetSafeToForceGpuSwitch();

  async_pixel_transfer_manager_.reset(
      AsyncPixelTransferManager::Create(context.get()));
  async_pixel_transfer_manager_->Initialize(texture_manager());

  framebuffer_manager()->AddObserver(this);

  return true;
}
