#ifndef R_OGL_H
#define R_OGL_H

#if OS_WINDOWS
# include "w32/r/ogl/w32_r_ogl.h"
#elif OS_LINUX
# include "lnx/r/ogl/lnx_r_ogl.h"
#else
# error OpenGL rendering backend not implemented for this operating system.
#endif

#include "r_ogl_core.h"

#endif // R_OGL_H
