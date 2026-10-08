#ifndef R_OGL_LNX_H
#define R_OGL_LNX_H

#define glActiveTexture glActiveTexture__static
#include <GL/gl.h>
#include <GL/glx.h>
#undef glActiveTexture

#define GLX_CONTEXT_MAJOR_VERSION_ARB          0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB          0x2092
#define GLX_CONTEXT_FLAGS_ARB                  0x2094
#define GLX_CONTEXT_DEBUG_BIT_ARB              0x00000001
#define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
typedef GLXContext (*glXCreateContextAttribsARB_Proc_Type)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

global GLXContext r_ogl_lnx_ctx = 0;

#endif // R_OGL_LNX_H
