#ifndef R_OGL_H
#define R_OGL_H

#if OS_WINDOWS
# include "r/ogl/w32/r_ogl_w32.h"
#elif OS_LINUX
# include "r/ogl/lnx/r_ogl_lnx.h"
#else
# error OpenGL rendering backend not implemented for this operating system.
#endif

#include "r_ogl_core.h"

#endif // R_OGL_H
