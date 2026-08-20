#include "render_core.c"
#if OS_WINDOWS
#include "d3d11/render_d3d11.c"
#else
# error Renderer backend not available.
#endif
