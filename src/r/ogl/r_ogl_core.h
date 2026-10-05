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

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF

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
  X(glEnableVertexAttribArray, void, (GLuint index))\
  X(glGetUniformLocation, GLint, (GLuint program, const GLchar *name))\
  X(glUniform2f, void, (GLint location, GLfloat v0, GLfloat v1))\
  X(glBufferSubData, void, (GLenum target, ptrdiff_t offset, ptrdiff_t size, const void *data))\
  X(glDrawArraysInstanced, void, (GLenum mode, GLint first, GLsizei count, GLsizei primcount))\
  X(glVertexAttribDivisor, void, (GLuint index, GLuint divisor))\
  X(glActiveTexture, void, (GLenum texture ))

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
