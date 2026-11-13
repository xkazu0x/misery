////////////////////////////////
// NOTE: Backend Hooks

internal void
r_ogl_init(void) {
  r_ogl_os_init();
#define X(name, r, p) name = (PFN_##name)r_ogl_os_load_proc(#name);
  R_OGL_PROC_XLIST
#undef X
}

internal GLenum
r_ogl_shader_stage(R_OGL_Shader_Stage stage) {
  GLenum result = 0;
  switch (stage) {
    case R_OGL_Shader_Stage_VERTEX:   result = GL_VERTEX_SHADER;   break;
    case R_OGL_Shader_Stage_FRAGMENT: result = GL_FRAGMENT_SHADER; break;
    default: invalid_path; break;
  }
  return(result);
}

internal GLuint
r_ogl_shader_compile(R_OGL_Shader_Stage stage, String8 *src) {
  GLuint shader = glCreateShader(r_ogl_shader_stage(stage));
  glShaderSource(shader, 1, (const GLchar **)&src->str, (const GLint *)&src->size);
  glCompileShader(shader);
  GLint compile_status = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  if (!compile_status) {
    GLint log_len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
      Temp scratch = scratch_begin(0, 0);
      GLchar *log_buf = push_array(scratch.arena, GLchar, log_len);
      glGetShaderInfoLog(shader, log_len, &log_len, log_buf);
      sh_message(1, str8_lit("OpenGL: Shader Compilation Error"), str8_make((u8 *)log_buf, log_len));
      scratch_end(scratch);
    }
    glDeleteShader(shader);
    shader = 0;
  }
  return(shader);
}

internal R_OGL_Shader
r_ogl_shader_make(String8 *vsrc, String8 *fsrc) {
  assert((vsrc && fsrc));

  GLuint vshader = r_ogl_shader_compile(R_OGL_Shader_Stage_VERTEX, vsrc);
  GLuint fshader = r_ogl_shader_compile(R_OGL_Shader_Stage_FRAGMENT, fsrc);

  GLuint program = glCreateProgram();
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  glLinkProgram(program);
  glDeleteShader(vshader);
  glDeleteShader(fshader);

  R_OGL_Shader result = {0};
  result.program = program;
  return(result);
}

internal void
r_ogl_shader_destroy(R_OGL_Shader *shader) {
  if (shader->program != 0) {
    glDeleteProgram(shader->program);
    shader->program = 0;
  }
}

internal GLenum
r_ogl_buffer_target(R_OGL_Buffer_Target target) {
  GLenum result = 0;
  switch (target) {
    case R_OGL_Buffer_Target_VERTEX: result = GL_ARRAY_BUFFER; break;
    case R_OGL_Buffer_Target_INDEX:  result = GL_ELEMENT_ARRAY_BUFFER; break;
    default: invalid_path; break;
  }
  return(result);
}

internal GLenum
r_ogl_buffer_usage(R_OGL_Buffer_Usage usage) {
  GLenum result = 0;
  switch (usage) {
    case R_OGL_Buffer_Usage_STATIC:  result = GL_STATIC_DRAW; break;
    case R_OGL_Buffer_Usage_DYNAMIC: result = GL_DYNAMIC_DRAW; break;
    case R_OGL_Buffer_Usage_STREAM:  result = GL_STREAM_DRAW; break;
    default: invalid_path; break;
  }
  return(result);
}

internal R_OGL_Buffer
r_ogl_buffer_make(R_OGL_Buffer_Target target, R_OGL_Buffer_Usage usage, u64 size, void *data) {
  GLenum ogl_target = r_ogl_buffer_target(target);
  GLenum ogl_usage  = r_ogl_buffer_usage(usage);

  GLuint id = 0;
  glGenBuffers(1, &id);
  glBindBuffer(ogl_target, id);
  glBufferData(ogl_target, size, 0, ogl_usage);
  if (data) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  R_OGL_Buffer result = {0};
  result.id = id;
  return(result);
}

internal void
r_ogl_buffer_destroy(R_OGL_Buffer *buffer) {
  if (buffer->id != 0) {
    glDeleteBuffers(1, &buffer->id);
    buffer->id = 0;
  }
}
