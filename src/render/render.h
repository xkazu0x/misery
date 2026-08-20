#ifndef RENDER_H
#define RENDER_H

#include "render_core.h"
#if OS_WINDOWS
#include "d3d11/render_d3d11.h"
#else
# error Renderer backend not available.
#endif

#endif // RENDER_H
