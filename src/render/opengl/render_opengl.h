#ifndef RENDER_OPENGL_H
#define RENDER_OPENGL_H

#if OS_WINDOWS
# include "render/opengl/win32/render_opengl_win32.h"
#elif OS_LINUX
# include "render/opengl/linux/render_opengl_linux.h"
#else
# error OpenGL rendering backend not implemented for this operating system.
#endif

#include "render_opengl_core.h"

#endif // RENDER_OPENGL_H
