#ifndef RENDER_D3D11_H
#define RENDER_D3D11_H

////////////////////////////////
// NOTE: Includes/Libraries

#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

////////////////////////////////
// NOTE: Shader Types

typedef enum R_D3D11_Vertex_Shader_Type {
  R_D3D11_Vertex_Shader_Type_RECT,
  R_D3D11_Vertex_Shader_Type_FINALIZE,
  R_D3D11_Vertex_Shader_Type_COUNT,
} R_D3D11_Vertex_Shader_Type;

typedef enum R_D3D11_Pixel_Shader_Type {
  R_D3D11_Pixel_Shader_Type_RECT,
  R_D3D11_Pixel_Shader_Type_FINALIZE,
  R_D3D11_Pixel_Shader_Type_COUNT,
} R_D3D11_Pixel_Shader_Type;

typedef enum R_D3D11_Uniform_Type {
  R_D3D11_Uniform_Type_RECT,
  R_D3D11_Uniform_Type_COUNT,
} R_D3D11_Uniform_Type;

typedef struct R_D3D11_Uniforms_Rect R_D3D11_Uniforms_Rect;
struct R_D3D11_Uniforms_Rect {
  Vector2 viewport_size_px;
  Vector2 texture_size_px;
};

////////////////////////////////
// NOTE: Shader Globals

global D3D11_INPUT_ELEMENT_DESC r_d3d11_g_rect_ilayout_elements[] = {
  {"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                            0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
  {"TEX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
  {"COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
  {"STY", 0, DXGI_FORMAT_R32_FLOAT,          0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
};

global String8 r_d3d11_g_rect_shader_src = str8_lit_comp(
  "cbuffer Uniforms: register(b0) {\n"
  "  float2 viewport_size_px;\n"
  "  float2 texture_size_px;\n"
  "}\n"
  "\n"
  "struct Vertex_Params {\n"
  "  float4 dst_rect_px: POS;\n"
  "  float4 src_rect_px: TEX;\n"
  "  float4 color:       COL;\n"
  "  float omit_texture: STY;\n"
  "  uint vertex_id:     SV_VertexID;\n"
  "};\n"
  "\n"
  "struct Pixel_Params {\n"
  "  float4 position: SV_POSITION;\n"
  "  float2 texcoord: TEX;\n"
  "  float4 color:    COL;\n"
  "  float omit_texture: STY;\n"
  "};\n"
  "\n"
  "Texture2D    main_texture: register(t0);"
  "SamplerState main_sampler: register(s0);"
  "\n"
  "Pixel_Params vs_main(Vertex_Params params) {\n"
  "  float2 dst_p0_px = params.dst_rect_px.xy;\n"
  "  float2 dst_p1_px = params.dst_rect_px.zw;\n"
  "  float2 src_p0_px = params.src_rect_px.xy;\n"
  "  float2 src_p1_px = params.src_rect_px.zw;\n"
  "  float2 dst_p_verts_px[] = {\n"
  "    float2(dst_p0_px.x, dst_p1_px.y),"
  "    float2(dst_p0_px.x, dst_p0_px.y),"
  "    float2(dst_p1_px.x, dst_p1_px.y),"
  "    float2(dst_p1_px.x, dst_p0_px.y),"
  "  };\n"
  "  float2 src_p_verts_px[] = {\n"
  "    float2(src_p0_px.x, src_p1_px.y),"
  "    float2(src_p0_px.x, src_p0_px.y),"
  "    float2(src_p1_px.x, src_p1_px.y),"
  "    float2(src_p1_px.x, src_p0_px.y),"
  "  };\n"
  "  Pixel_Params result = (Pixel_Params)0;\n"
  "  float2 p_vert_px = dst_p_verts_px[params.vertex_id];\n"
  "  p_vert_px.y = viewport_size_px.y - p_vert_px.y;\n"
  "  result.position.xy = 2.f*p_vert_px/viewport_size_px - 1.f;\n"
  "  result.position.z = 0.f;\n"
  "  result.position.w = 1.f;\n"
  "  result.texcoord = src_p_verts_px[params.vertex_id]/texture_size_px;\n"
  "  result.color = params.color;\n"
  "  result.omit_texture = params.omit_texture;\n"
  "  return(result);\n"
  "}\n"
  "\n"
  "float4 ps_main(Pixel_Params params): SV_TARGET {\n"
  "  float4 final_color = float4(1.f, 1.f, 1.f, 1.f);"
  "  if (params.omit_texture < 1.f) {\n"
  "    final_color = main_texture.Sample(main_sampler, params.texcoord);\n"
  "  }\n"
  "  final_color *= params.color;\n"
  "  return(final_color);\n"
  "}\n"
);

global String8 r_d3d11_g_finalize_shader_src = str8_lit_comp(
  "struct Vertex_Params {\n"
  "  uint vertex_id: SV_VertexID;\n"
  "};\n"
  "\n"
  "struct Pixel_Params{\n"
  "  float4 position: SV_POSITION;\n"
  "  float2 texcoord: TEX;\n"
  "};\n"
  "\n"
  "Texture2D    stage_texture: register(t0);\n"
  "SamplerState stage_sampler: register(s0);\n"
  "\n"
  "Pixel_Params\n"
  "vs_main(Vertex_Params params){\n"
  "  float4 vertex_positions__modl[] = {\n"
  "    float4(0, 0, 0, 1),\n"
  "    float4(0, 1, 0, 1),\n"
  "    float4(1, 0, 0, 1),\n"
  "    float4(1, 1, 0, 1),\n"
  "  };\n"
  "  float4 vertex_position__modl = vertex_positions__modl[params.vertex_id];\n"
  "  float4 vertex_position__clip = float4(2*vertex_position__modl.x - 1, 2*vertex_position__modl.y - 1, 0, 1);\n"
  "  float2 texcoord = float2(vertex_position__modl.x, vertex_position__modl.y);\n"
  "  texcoord.y = 1-texcoord.y;\n"
  "  Pixel_Params result;\n"
  "  result.position = vertex_position__clip;\n"
  "  result.texcoord = texcoord;\n"
  "  return(result);\n"
  "}\n"
  "\n"
  "float4\n"
  "ps_main(Pixel_Params params): SV_TARGET {\n"
  "  float4 final_color = stage_texture.Sample(stage_sampler, params.texcoord);\n"
  "  final_color.a = 1;\n"
  "  return(final_color);\n"
  "}\n"
);

String8 *r_d3d11_g_vshader_type_source_table[R_D3D11_Vertex_Shader_Type_COUNT] = {
  &r_d3d11_g_rect_shader_src,
  &r_d3d11_g_finalize_shader_src,
};

D3D11_INPUT_ELEMENT_DESC *r_d3d11_g_vshader_type_elements_ptr_table[R_D3D11_Vertex_Shader_Type_COUNT] = {
  r_d3d11_g_rect_ilayout_elements,
  0,
};

u64 r_d3d11_g_vshader_type_elements_count_table[R_D3D11_Vertex_Shader_Type_COUNT] = {
  array_count(r_d3d11_g_rect_ilayout_elements),
  0,
};

String8 *r_d3d11_g_pshader_type_source_table[R_D3D11_Pixel_Shader_Type_COUNT] = {
  &r_d3d11_g_rect_shader_src,
  &r_d3d11_g_finalize_shader_src,
};

u64 r_d3d11_g_uniform_type_size_table[R_D3D11_Uniform_Type_COUNT] = {
  sizeof(R_D3D11_Uniforms_Rect),
};

////////////////////////////////
// NOTE: Main State Types

typedef struct R_D3D11_Window R_D3D11_Window;
struct R_D3D11_Window {
  R_D3D11_Window *next;

  // NOTE: swapchain/framebuffer
  IDXGISwapChain1          *swapchain;
  ID3D11Texture2D          *framebuffer;
  ID3D11RenderTargetView   *framebuffer_rtv;

  // NOTE: staging buffer
  ID3D11Texture2D          *stage_color;
  ID3D11RenderTargetView   *stage_color_rtv;
  ID3D11ShaderResourceView *stage_color_srv;

  // NOTE: last state
  Vector2 last_resolution;
};

typedef struct R_D3D11_Texture R_D3D11_Texture;
struct R_D3D11_Texture {
  R_D3D11_Texture *next;
  ID3D11Texture2D          *tex;
  ID3D11ShaderResourceView *srv;
  R_Resource_Type type;
  Vector2 size;
};

typedef struct R_D3D11_Flush_Buffer R_D3D11_Flush_Buffer;
struct R_D3D11_Flush_Buffer {
  R_D3D11_Flush_Buffer *next;
  ID3D11Buffer *buffer;
};

typedef struct R_D3D11_State R_D3D11_State;
struct R_D3D11_State {
  // NOTE: state
  Arena *arena;
  R_D3D11_Window *first_free_window;
  R_D3D11_Texture *first_free_texture;
  R_D3D11_Texture *first_to_free_texture;

  // NOTE: base d3d11 objects
  ID3D11Device            *base_device;
  ID3D11DeviceContext     *base_device_ctx;
  ID3D11Device1           *device;
  ID3D11DeviceContext1    *device_ctx;
  IDXGIDevice1            *dxgi_device;
  IDXGIAdapter            *dxgi_adapter;
  IDXGIFactory2           *dxgi_factory;
  ID3D11RasterizerState   *main_rasterizer;
  ID3D11BlendState        *main_blend_state;
  ID3D11SamplerState      *nearest_sampler;
  ID3D11DepthStencilState *noop_depth_stencil;
  ID3D11Buffer            *instance_scratch_buffer_64kb;

  // NOTE: backup texture
  R_Texture backup_texture;

  // NOTE: shaders/uniforms
  ID3D11VertexShader *vshaders[R_D3D11_Vertex_Shader_Type_COUNT];
  ID3D11InputLayout  *ilayouts[R_D3D11_Vertex_Shader_Type_COUNT];
  ID3D11PixelShader  *pshaders[R_D3D11_Pixel_Shader_Type_COUNT];
  ID3D11Buffer       *uniforms[R_D3D11_Uniform_Type_COUNT];

  // NOTE: buffers to flush at subsequent frame
  Arena *buffer_flush_arena;
  R_D3D11_Flush_Buffer *first_buffer_to_flush;
  R_D3D11_Flush_Buffer *last_buffer_to_flush;
};

////////////////////////////////
// NOTE: Globals

global R_D3D11_State *r_d3d11_state = 0;

////////////////////////////////
// NOTE: Helpers

internal R_Window         r_d3d11_handle_from_window(R_D3D11_Window *window);
internal R_D3D11_Window  *r_d3d11_window_from_handle(R_Window handle);
internal R_Texture        r_d3d11_handle_from_texture(R_D3D11_Texture *texture);
internal R_D3D11_Texture *r_d3d11_texture_from_handle(R_Texture handle);
internal void             r_d3d11_usage_and_cpu_access_flags_from_resource_type(R_Resource_Type type, D3D11_USAGE *out_usage, UINT *out_cpu_access_flags);
internal ID3D11Buffer    *r_d3d11_instance_buffer_from_size(u64 size);

#endif // RENDER_D3D11_H
