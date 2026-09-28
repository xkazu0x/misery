#ifndef R_OGL_CORE_H
#define R_OGL_CORE_H

////////////////////////////////
// NOTE: Defines

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

#define GL_ARRAY_BUFFER                   0x8892
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84

////////////////////////////////
// NOTE: OpenGL Proc List

#define R_OGL_PROC_XLIST\
  X(glCreateShader, GLuint, (GLenum shaderType))\
  X(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar **string, const GLint *length))\
  X(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params))\
  X(glGetShaderInfoLog, void, (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog))\
  X(glCompileShader, void, (GLuint shader))\
  X(glDeleteShader, void, (GLuint shader))\
  X(glCreateProgram, GLuint, (void))\
  X(glAttachShader, void, (GLuint program, GLuint shader))\
  X(glLinkProgram, void, (GLuint program))\
  X(glValidateProgram, void, (GLuint program))\
  X(glUseProgram, void, (GLuint program))\
  X(glGenVertexArrays, void, (GLsizei n, GLuint *arrays))\
  X(glBindVertexArray, void, (GLuint array))\
  X(glGenBuffers, void, (GLsizei n, GLuint *buffers))\
  X(glBindBuffer, void, (GLenum target, GLuint buffer))\
  X(glBufferData, void, (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage))\
  X(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer))\
  X(glEnableVertexAttribArray, void, (GLuint index))

#define X(name, r, p) typedef r name##_Proc_Type p;
R_OGL_PROC_XLIST;
#undef X
#define X(name, r, p) global name##_Proc_Type *name = 0;
R_OGL_PROC_XLIST;
#undef X

////////////////////////////////
// NOTE: @per_os_impl Hooks

internal void_proc *r_ogl_load_proc(char *name);
internal void r_ogl_init(void);
internal void r_ogl_window_equip(WM_Window window);
internal void r_ogl_window_unequip(WM_Window window);
internal void r_ogl_window_select(WM_Window window);
internal void r_ogl_window_swap(WM_Window window);

#endif // R_OGL_CORE_H
