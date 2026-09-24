TextureManager::TextureInfo::Ref TextureManager::CreateDefaultAndBlackTextures(
    GLenum target,
    GLuint* black_texture) {
  static uint8 black[] = {0, 0, 0, 255};

  bool needs_initialization = (target != GL_TEXTURE_EXTERNAL_OES);
  bool needs_faces = (target == GL_TEXTURE_CUBE_MAP);

  GLuint ids[2];
  glGenTextures(arraysize(ids), ids);
  for (unsigned long ii = 0; ii < arraysize(ids); ++ii) {
    glBindTexture(target, ids[ii]);
    if (needs_initialization) {
      if (needs_faces) {
        for (int jj = 0; jj < GLES2Util::kNumFaces; ++jj) {
          glTexImage2D(GLES2Util::IndexToGLFaceTarget(jj), 0, GL_RGBA, 1, 1, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, black);
        }
      } else {
        glTexImage2D(target, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, black);
      }
    }
  }
  glBindTexture(target, 0);

  ++num_unrenderable_textures_;
  TextureInfo::Ref default_texture = TextureInfo::Ref(
      new TextureInfo(this, ids[1]));
  SetInfoTarget(default_texture, target);
  if (needs_faces) {
    for (int ii = 0; ii < GLES2Util::kNumFaces; ++ii) {
      SetLevelInfo(
          default_texture, GLES2Util::IndexToGLFaceTarget(ii),
          0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, true);
    }
  } else {
    if (needs_initialization) {
      SetLevelInfo(default_texture,
                   GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 1, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, true);
    } else {
      SetLevelInfo(
          default_texture, GL_TEXTURE_EXTERNAL_OES, 0,
          GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, true);
    }
  }

  *black_texture = ids[0];
  return default_texture;
}
