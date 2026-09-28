#if OS_WINDOWS
# include "w32/r/ogl/w32_r_ogl.c"
#elif OS_LINUX
# include "lnx/r/ogl/lnx_r_ogl.c"
#else
# error OpenGL rendering backend not implemented for this operating system.
#endif

#include "r_ogl_core.c"
