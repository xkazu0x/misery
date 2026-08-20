////////////////////////////////
// NOTE: Helpers

internal R_Window
r_d3d11_handle_from_window(R_D3D11_Window *window) {
  R_Window result = {0};
  result.v[0] = (u64)window;
  return(result);
}

internal R_D3D11_Window *
r_d3d11_window_from_handle(R_Window handle) {
  R_D3D11_Window *result = (R_D3D11_Window *)handle.v[0];
  return(result);
}

internal R_Texture
r_d3d11_handle_from_texture(R_D3D11_Texture *texture) {
  R_Texture result = {0};
  result.v[0] = (u64)texture;
  return(result);
}

internal R_D3D11_Texture *
r_d3d11_texture_from_handle(R_Texture handle) {
  R_D3D11_Texture *result = (R_D3D11_Texture *)handle.v[0];
  return(result);
}

internal void
r_d3d11_usage_and_cpu_access_flags_from_resource_type(R_Resource_Type type, D3D11_USAGE *out_usage, UINT *out_cpu_access_flags) {
  switch (type) {
    case R_Resource_Type_STATIC: {
      *out_usage = D3D11_USAGE_IMMUTABLE;
      *out_cpu_access_flags = 0;
    } break;
    case R_Resource_Type_DYNAMIC: {
      *out_usage = D3D11_USAGE_DEFAULT;
      *out_cpu_access_flags = 0;
    } break;
    case R_Resource_Type_STREAM: {
      *out_usage = D3D11_USAGE_DEFAULT;
      *out_cpu_access_flags = D3D11_CPU_ACCESS_WRITE;
    } break;
    default: {
      invalid_path;
    } break;
  }
}

internal ID3D11Buffer *
r_d3d11_instance_buffer_from_size(u64 size) {
  ID3D11Buffer *buffer = r_d3d11_state->instance_scratch_buffer_64kb;
  if (size > KB(64)) {
    u64 flushed_buffer_size = size;
    flushed_buffer_size += MB(1)-1;
    flushed_buffer_size -= flushed_buffer_size % MB(1);

    // NOTE: build buffer
    D3D11_BUFFER_DESC desc = {0};
    desc.ByteWidth      = (UINT)flushed_buffer_size;
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HRESULT error = r_d3d11_state->device->lpVtbl->CreateBuffer(r_d3d11_state->device, &desc, 0, &buffer);
    (void)error;

    // NOTE: push buffer to flush list
    R_D3D11_Flush_Buffer *n = push_array(r_d3d11_state->buffer_flush_arena, R_D3D11_Flush_Buffer, 1);
    n->buffer;
    sll_queue_push(r_d3d11_state->first_buffer_to_flush, r_d3d11_state->last_buffer_to_flush, n);
  }
  return(buffer);
}

////////////////////////////////
// NOTE: Backend Hooks

// NOTE: top-level layer initialization

internal void
r_init(void) {
  Arena *arena = arena_alloc();
  r_d3d11_state = push_array(arena, R_D3D11_State, 1);
  r_d3d11_state->arena = arena;
  HRESULT error = 0;

  // NOTE: create base device
  {
    UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if BUILD_DEBUG
    creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0};
    D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_HARDWARE;
    error = D3D11CreateDevice(0,
                              driver_type,
                              0,
                              creation_flags,
                              feature_levels,
                              array_count(feature_levels),
                              D3D11_SDK_VERSION,
                              &r_d3d11_state->base_device,
                              0,
                              &r_d3d11_state->base_device_ctx);
    if (FAILED(error)) {
      Temp scratch = scratch_begin(0, 0);
      String8 message = str8f(scratch.arena, "D3D11 device creation failure (%lx). The process is terminating.", error);
      wm_graphical_message(1, str8_lit("Fatal Error"), message);
      scratch_end(scratch);
      abort_self(1);
    }
  }

  // NOTE: enable break-on-error
#if BUILD_DEBUG
  {
    ID3D11InfoQueue* info = 0;
    error = r_d3d11_state->base_device->lpVtbl->QueryInterface(r_d3d11_state->base_device, &IID_ID3D11InfoQueue, (void **)(&info));
    if (SUCCEEDED(error)) {
      error = info->lpVtbl->SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
      error = info->lpVtbl->SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
      info->lpVtbl->Release(info);
    }
  }
#endif

  // NOTE: get main device
  error = r_d3d11_state->base_device->lpVtbl->QueryInterface(r_d3d11_state->base_device, &IID_ID3D11Device1, (void **)(&r_d3d11_state->device));
  error = r_d3d11_state->base_device_ctx->lpVtbl->QueryInterface(r_d3d11_state->base_device_ctx, &IID_ID3D11DeviceContext1, (void **)(&r_d3d11_state->device_ctx));

  // NOTE: get dxgi device/adapter/factory
  error = r_d3d11_state->device->lpVtbl->QueryInterface(r_d3d11_state->device, &IID_IDXGIDevice1, (void **)(&r_d3d11_state->dxgi_device));
  error = r_d3d11_state->dxgi_device->lpVtbl->GetAdapter(r_d3d11_state->dxgi_device, &r_d3d11_state->dxgi_adapter);
  error = r_d3d11_state->dxgi_adapter->lpVtbl->GetParent(r_d3d11_state->dxgi_adapter, &IID_IDXGIFactory2, (void **)(&r_d3d11_state->dxgi_factory));
  error = r_d3d11_state->dxgi_device->lpVtbl->SetMaximumFrameLatency(r_d3d11_state->dxgi_device, 1);

  // NOTE: create main rasterizer
  {
    D3D11_RASTERIZER_DESC desc = {0};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;
    desc.ScissorEnable = 1;
    error = r_d3d11_state->device->lpVtbl->CreateRasterizerState(r_d3d11_state->device, &desc, &r_d3d11_state->main_rasterizer);
  }

  // NOTE: create main blend state
  {
    D3D11_BLEND_DESC desc = {0};
    desc.RenderTarget[0].BlendEnable           = 1;
    desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    error = r_d3d11_state->device->lpVtbl->CreateBlendState(r_d3d11_state->device, &desc, &r_d3d11_state->main_blend_state);
  }

  // NOTE: create nearest-neighbor sampler
  {
    D3D11_SAMPLER_DESC desc = {0};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    error = r_d3d11_state->device->lpVtbl->CreateSamplerState(r_d3d11_state->device, &desc, &r_d3d11_state->nearest_sampler);
  }

  // NOTE: create noop depth/stencil state
  {
    D3D11_DEPTH_STENCIL_DESC desc = {0};
    desc.DepthEnable    = FALSE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc      = D3D11_COMPARISON_LESS;
    error = r_d3d11_state->device->lpVtbl->CreateDepthStencilState(r_d3d11_state->device, &desc, &r_d3d11_state->noop_depth_stencil);
  }

  // NOTE: create buffers
  {
    D3D11_BUFFER_DESC desc = {0};
    desc.ByteWidth      = KB(64);
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    error = r_d3d11_state->device->lpVtbl->CreateBuffer(r_d3d11_state->device, &desc, 0, &r_d3d11_state->instance_scratch_buffer_64kb);
  }

  // NOTE: build vertex shaders & input layouts
  for (R_D3D11_Vertex_Shader_Type type = (R_D3D11_Vertex_Shader_Type)0; type < R_D3D11_Vertex_Shader_Type_COUNT; type = (R_D3D11_Vertex_Shader_Type)(type+1)) {
    String8 source = *r_d3d11_g_vshader_type_source_table[type];
    D3D11_INPUT_ELEMENT_DESC *ilayout_elements = r_d3d11_g_vshader_type_elements_ptr_table[type];
    u64 ilayout_count = r_d3d11_g_vshader_type_elements_count_table[type];

    UINT compilation_flags = D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if BUILD_DEBUG
    compilation_flags |= D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compilation_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    // NOTE: compile vertex shader
    ID3DBlob *vshader_source_blob = 0;
    ID3DBlob *vshader_source_errors = 0;
    ID3D11VertexShader *vshader = 0;
    error = D3DCompile(source.str,
                       source.size,
                       0,
                       0,
                       0,
                       "vs_main",
                       "vs_5_0",
                       compilation_flags,
                       0,
                       &vshader_source_blob,
                       &vshader_source_errors);
    if (FAILED(error)) {
      String8 errors = str8_make(vshader_source_errors->lpVtbl->GetBufferPointer(vshader_source_errors), vshader_source_errors->lpVtbl->GetBufferSize(vshader_source_errors));
      wm_graphical_message(1, str8_lit("Vertex Shader Compilation Failure"), errors);
    } else {
      error = r_d3d11_state->device->lpVtbl->CreateVertexShader(r_d3d11_state->device, vshader_source_blob->lpVtbl->GetBufferPointer(vshader_source_blob), vshader_source_blob->lpVtbl->GetBufferSize(vshader_source_blob), 0, &vshader);
    }

    // NOTE: make input layout
    ID3D11InputLayout *ilayout = 0;
    if (ilayout_elements != 0) {
      error = r_d3d11_state->device->lpVtbl->CreateInputLayout(r_d3d11_state->device, ilayout_elements, (UINT)ilayout_count, vshader_source_blob->lpVtbl->GetBufferPointer(vshader_source_blob), vshader_source_blob->lpVtbl->GetBufferSize(vshader_source_blob), &ilayout);
    }

    vshader_source_blob->lpVtbl->Release(vshader_source_blob);

    // NOTE: store
    r_d3d11_state->vshaders[type] = vshader;
    r_d3d11_state->ilayouts[type] = ilayout;
  }

  // NOTE: build pixel shaders
  for (R_D3D11_Pixel_Shader_Type type = (R_D3D11_Pixel_Shader_Type)0; type < R_D3D11_Pixel_Shader_Type_COUNT; type = (R_D3D11_Pixel_Shader_Type)(type+1)) {
    String8 source = *r_d3d11_g_pshader_type_source_table[type];

    UINT compilation_flags = D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if BUILD_DEBUG
    compilation_flags |= D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compilation_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    // NOTE: compile pixel shader
    ID3DBlob *pshader_source_blob = 0;
    ID3DBlob *pshader_source_errors = 0;
    ID3D11PixelShader *pshader = 0;
    error = D3DCompile(source.str,
                       source.size,
                       0,
                       0,
                       0,
                       "ps_main",
                       "ps_5_0",
                       compilation_flags,
                       0,
                       &pshader_source_blob,
                       &pshader_source_errors);
    if (FAILED(error)) {
      String8 errors = str8_make(pshader_source_errors->lpVtbl->GetBufferPointer(pshader_source_errors), pshader_source_errors->lpVtbl->GetBufferSize(pshader_source_errors));
      wm_graphical_message(1, str8_lit("Pixel Shader Compilation Failure"), errors);
    } else {
      error = r_d3d11_state->device->lpVtbl->CreatePixelShader(r_d3d11_state->device, pshader_source_blob->lpVtbl->GetBufferPointer(pshader_source_blob), pshader_source_blob->lpVtbl->GetBufferSize(pshader_source_blob), 0, &pshader);
    }

    pshader_source_blob->lpVtbl->Release(pshader_source_blob);

    // NOTE: store
    r_d3d11_state->pshaders[type] = pshader;
  }

  // NOTE: build uniform type buffers
  for (R_D3D11_Uniform_Type type = (R_D3D11_Uniform_Type)0; type < R_D3D11_Uniform_Type_COUNT; type = (R_D3D11_Uniform_Type)(type+1)) {
    ID3D11Buffer *buffer = 0;
    {
      D3D11_BUFFER_DESC desc = {0};
      desc.ByteWidth      = (UINT)r_d3d11_g_uniform_type_size_table[type];
      desc.ByteWidth     += 15;
      desc.ByteWidth     -= desc.ByteWidth % 16;
      desc.Usage          = D3D11_USAGE_DYNAMIC;
      desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      r_d3d11_state->device->lpVtbl->CreateBuffer(r_d3d11_state->device, &desc, 0, &buffer);
    }
    r_d3d11_state->uniforms[type] = buffer;
  }

  // NOTE: create backup texture
  {
    u32 backup_texture_data[] = {
      0xff000000, 0xffff00ff,
      0xffff00ff, 0xff000000,
    };

    r_d3d11_state->backup_texture = r_texture_alloc(R_Resource_Type_STATIC, vector2_make(2, 2), backup_texture_data);
  }

  // NOTE: init buffer flush state
  {
    r_d3d11_state->buffer_flush_arena = arena_alloc();
  }
}

// NOTE: windows

internal R_Window
r_window_equip(WM_Window handle) {
  R_D3D11_Window *window = r_d3d11_state->first_free_window;
  if (window) {
    sll_stack_pop(r_d3d11_state->first_free_window);
  } else {
    window = push_array(r_d3d11_state->arena, R_D3D11_Window, 1);
  }
  memory_zero_struct(window);

  // NOTE: get hwnd from window
  HWND hwnd = {0};
  {
    W32_WM_Window *w32_layer_window = w32_wm_window_from_handle(handle);
    hwnd = w32_wm_hwnd_from_window(w32_layer_window);
  }

  // create swap chain
  DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {0};
  swapchain_desc.Width              = 0; // NOTE: use window width
  swapchain_desc.Height             = 0; // NOTE: use window height
  swapchain_desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
  swapchain_desc.Stereo             = FALSE;
  swapchain_desc.SampleDesc.Count   = 1;
  swapchain_desc.SampleDesc.Quality = 0;
  swapchain_desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapchain_desc.BufferCount        = 2;
  swapchain_desc.Scaling            = DXGI_SCALING_NONE;
  swapchain_desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapchain_desc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
  swapchain_desc.Flags              = 0;

  HRESULT error = r_d3d11_state->dxgi_factory->lpVtbl->CreateSwapChainForHwnd(r_d3d11_state->dxgi_factory, (IUnknown *)r_d3d11_state->device, hwnd, &swapchain_desc, 0, 0, &window->swapchain);
  if (FAILED(error)) {
    Temp scratch = scratch_begin(0, 0);
    String8 message = str8f(scratch.arena, "D3D11 swapchain creation failure (%lx). The process is terminating.", error);
    wm_graphical_message(1, str8_lit("Fatal Error"), message);
    scratch_end(scratch);
    abort_self(1);
  }

  r_d3d11_state->dxgi_factory->lpVtbl->MakeWindowAssociation(r_d3d11_state->dxgi_factory, hwnd, DXGI_MWA_NO_ALT_ENTER);

  // NOTE: create framebuffer & view
  {
    D3D11_RENDER_TARGET_VIEW_DESC framebuffer_rtv_desc = {0};
    framebuffer_rtv_desc.Format        = DXGI_FORMAT_B8G8R8A8_UNORM;
    framebuffer_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    window->swapchain->lpVtbl->GetBuffer(window->swapchain, 0, &IID_ID3D11Texture2D, (void **)(&window->framebuffer));
    r_d3d11_state->device->lpVtbl->CreateRenderTargetView(r_d3d11_state->device, (ID3D11Resource *)window->framebuffer, &framebuffer_rtv_desc, &window->framebuffer_rtv);
  }

  // NOTE: create stage color targets
  {
    D3D11_TEXTURE2D_DESC color_desc = {0};
    window->framebuffer->lpVtbl->GetDesc(window->framebuffer, &color_desc);
    color_desc.Format    = DXGI_FORMAT_R16G16B16A16_FLOAT;
    color_desc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;

    D3D11_RENDER_TARGET_VIEW_DESC color_rtv_desc = {0};
    color_rtv_desc.Format        = color_desc.Format;
    color_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    D3D11_SHADER_RESOURCE_VIEW_DESC color_srv_desc = {0};
    color_srv_desc.Format              = DXGI_FORMAT_R16G16B16A16_FLOAT;
    color_srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
    color_srv_desc.Texture2D.MipLevels = (UINT)-1;

    r_d3d11_state->device->lpVtbl->CreateTexture2D(r_d3d11_state->device, &color_desc, 0, &window->stage_color);
    r_d3d11_state->device->lpVtbl->CreateRenderTargetView(r_d3d11_state->device, (ID3D11Resource *)window->stage_color, &color_rtv_desc, &window->stage_color_rtv);
    r_d3d11_state->device->lpVtbl->CreateShaderResourceView(r_d3d11_state->device, (ID3D11Resource *)window->stage_color, &color_srv_desc, &window->stage_color_srv);
  }

  R_Window result = r_d3d11_handle_from_window(window);
  return(result);
}

internal void
r_window_unequip(WM_Window handle, R_Window equip_handle) {
  R_D3D11_Window *window = r_d3d11_window_from_handle(equip_handle);
  window->stage_color_srv->lpVtbl->Release(window->stage_color_srv);
  window->stage_color_rtv->lpVtbl->Release(window->stage_color_rtv);
  window->stage_color->lpVtbl->Release(window->stage_color);
  window->framebuffer_rtv->lpVtbl->Release(window->framebuffer_rtv);
  window->framebuffer->lpVtbl->Release(window->framebuffer);
  window->swapchain->lpVtbl->Release(window->swapchain);
  sll_stack_push(r_d3d11_state->first_free_window, window);
}

// NOTE: textures

internal R_Texture
r_texture_alloc(R_Resource_Type type, Vector2 size, void *data) {
  R_D3D11_Texture *texture = r_d3d11_state->first_free_texture;
  if (texture) {
    sll_stack_pop(r_d3d11_state->first_free_texture);
  } else {
    texture = push_array(r_d3d11_state->arena, R_D3D11_Texture, 1);
  }
  memory_zero_struct(texture);

  D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
  UINT cpu_access_flags = 0;
  r_d3d11_usage_and_cpu_access_flags_from_resource_type(type, &usage, &cpu_access_flags);

  D3D11_TEXTURE2D_DESC desc = {0};
  desc.Width            = (UINT)size.x;
  desc.Height           = (UINT)size.y;
  desc.MipLevels        = 1;
  desc.ArraySize        = 1;
  desc.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage            = usage;
  desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags   = cpu_access_flags;

  D3D11_SUBRESOURCE_DATA sub_rsrc_ = {0};
  D3D11_SUBRESOURCE_DATA *sub_rsrc = 0;
  if (data != 0) {
    sub_rsrc = &sub_rsrc_;
    sub_rsrc->pSysMem = data;
    sub_rsrc->SysMemPitch = (UINT)(size.x*sizeof(u32));
  }

  r_d3d11_state->device->lpVtbl->CreateTexture2D(r_d3d11_state->device, &desc, sub_rsrc, &texture->tex);
  r_d3d11_state->device->lpVtbl->CreateShaderResourceView(r_d3d11_state->device, (ID3D11Resource *)texture->tex, 0, &texture->srv);

  texture->type = type;
  texture->size = size;

  R_Texture result = r_d3d11_handle_from_texture(texture);
  return(result);
}

internal void
r_texture_release(R_Texture handle) {
  R_D3D11_Texture *texture = r_d3d11_texture_from_handle(handle);
  if (texture != 0) {
    sll_stack_push(r_d3d11_state->first_to_free_texture, texture);
  }
}

internal Vector2
r_texture_size(R_Texture handle) {
  Vector2 result = {0};
  R_D3D11_Texture *texture = r_d3d11_texture_from_handle(handle);
  if (texture != 0) {
    result = texture->size;
  }
  return(result);
}

// NOTE: frame markers

internal void
r_begin(WM_Window handle, R_Window equip_handle) {
  R_D3D11_Window *window = r_d3d11_window_from_handle(equip_handle);
  ID3D11DeviceContext1 *d_ctx = r_d3d11_state->device_ctx;

  // NOTE: get resolution
  Range2 client_rect = wm_client_rect_from_window(handle);
  Vector2 resolution = range2_dim(client_rect);

  // NOTE: resolution change
  b32 resize_done = 0;
  if (window->last_resolution.x != resolution.x || window->last_resolution.y != resolution.y) {
    resize_done = 1;
    window->last_resolution = resolution;

    // NOTE: release screen-sized render target resources, if there
    window->stage_color_srv->lpVtbl->Release(window->stage_color_srv);
    window->stage_color_rtv->lpVtbl->Release(window->stage_color_rtv);
    window->stage_color->lpVtbl->Release(window->stage_color);

    // NOTE: resize swapchain & main framebuffer
    window->framebuffer_rtv->lpVtbl->Release(window->framebuffer_rtv);
    window->framebuffer->lpVtbl->Release(window->framebuffer);
    window->swapchain->lpVtbl->ResizeBuffers(window->swapchain, 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    window->swapchain->lpVtbl->GetBuffer(window->swapchain, 0, &IID_ID3D11Texture2D, (void **)(&window->framebuffer));
    D3D11_RENDER_TARGET_VIEW_DESC framebuffer_rtv_desc = {0};
    framebuffer_rtv_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    framebuffer_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    r_d3d11_state->device->lpVtbl->CreateRenderTargetView(r_d3d11_state->device, (ID3D11Resource *)window->framebuffer, &framebuffer_rtv_desc, &window->framebuffer_rtv);

    // NOTE: create stage color targets
    {
      D3D11_TEXTURE2D_DESC color_desc = {0};
      window->framebuffer->lpVtbl->GetDesc(window->framebuffer, &color_desc);
      color_desc.Format    = DXGI_FORMAT_R16G16B16A16_FLOAT;
      color_desc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;

      D3D11_RENDER_TARGET_VIEW_DESC color_rtv_desc = {0};
      color_rtv_desc.Format        = color_desc.Format;
      color_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

      D3D11_SHADER_RESOURCE_VIEW_DESC color_srv_desc = {0};
      color_srv_desc.Format              = DXGI_FORMAT_R16G16B16A16_FLOAT;
      color_srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
      color_srv_desc.Texture2D.MipLevels = (UINT)-1;

      r_d3d11_state->device->lpVtbl->CreateTexture2D(r_d3d11_state->device, &color_desc, 0, &window->stage_color);
      r_d3d11_state->device->lpVtbl->CreateRenderTargetView(r_d3d11_state->device, (ID3D11Resource *)window->stage_color, &color_rtv_desc, &window->stage_color_rtv);
      r_d3d11_state->device->lpVtbl->CreateShaderResourceView(r_d3d11_state->device, (ID3D11Resource *)window->stage_color, &color_srv_desc, &window->stage_color_srv);
    }
  }

  Vector4 clear_color = {0.2f, 0.3f, 0.3f, 1.0f};
  d_ctx->lpVtbl->ClearRenderTargetView(d_ctx, window->framebuffer_rtv, clear_color.v);
  d_ctx->lpVtbl->ClearRenderTargetView(d_ctx, window->stage_color_rtv, clear_color.v);
  if (resize_done) {
    d_ctx->lpVtbl->Flush(d_ctx);
  }
}

internal void
r_end(WM_Window handle, R_Window equip_handle) {
  R_D3D11_Window *window = r_d3d11_window_from_handle(equip_handle);
  ID3D11DeviceContext1 *d_ctx = r_d3d11_state->device_ctx;

  ID3D11VertexShader *vshader = r_d3d11_state->vshaders[R_D3D11_Vertex_Shader_Type_FINALIZE];
  ID3D11PixelShader  *pshader = r_d3d11_state->pshaders[R_D3D11_Pixel_Shader_Type_FINALIZE];

  // NOTE: setup output merger
  d_ctx->lpVtbl->OMSetRenderTargets(d_ctx, 1, &window->framebuffer_rtv, 0);
  d_ctx->lpVtbl->OMSetDepthStencilState(d_ctx, r_d3d11_state->noop_depth_stencil, 0);
  d_ctx->lpVtbl->OMSetBlendState(d_ctx, r_d3d11_state->main_blend_state, 0, 0xffffffff);

  // NOTE: setup rasterizer
  Vector2 resolution = window->last_resolution;
  D3D11_VIEWPORT viewport = {0.0f, 0.0f, resolution.x, resolution.y, 0.0f, 1.0f};
  d_ctx->lpVtbl->RSSetViewports(d_ctx, 1, &viewport);
  d_ctx->lpVtbl->RSSetState(d_ctx, r_d3d11_state->main_rasterizer);

  // NOTE: setup input assembly
  d_ctx->lpVtbl->IASetPrimitiveTopology(d_ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  d_ctx->lpVtbl->IASetInputLayout(d_ctx, 0);

  // NOTE: setup shaders
  d_ctx->lpVtbl->VSSetShader(d_ctx, vshader, 0, 0);
  d_ctx->lpVtbl->PSSetShader(d_ctx, pshader, 0, 0);
  d_ctx->lpVtbl->PSSetShaderResources(d_ctx, 0, 1, &window->stage_color_srv);
  d_ctx->lpVtbl->PSSetSamplers(d_ctx, 0, 1, &r_d3d11_state->nearest_sampler);

  // NOTE: setup scissor rect
  D3D11_RECT rect = {0};
  rect.left   = 0;
  rect.top    = 0;
  rect.right  = (LONG)resolution.x;
  rect.bottom = (LONG)resolution.y;
  d_ctx->lpVtbl->RSSetScissorRects(d_ctx, 1, &rect);

  // NOTE: draw
  d_ctx->lpVtbl->Draw(d_ctx, 4, 0);

  // NOTE: present
  HRESULT error = window->swapchain->lpVtbl->Present(window->swapchain, 1, 0);
  if (FAILED(error)) {
    Temp scratch = scratch_begin(0, 0);
    String8 message = str8f(scratch.arena, "D3D11 present failure (%lx). The process is terminating.", error);
    wm_graphical_message(1, str8_lit("Fatal Error"), message);
    scratch_end(scratch);
    abort_self(1);
  }

  d_ctx->lpVtbl->ClearState(d_ctx);

  // NOTE: flush buffers
  for (R_D3D11_Flush_Buffer *n = r_d3d11_state->first_buffer_to_flush; n != 0; n = n->next) {
    if (n->buffer != 0) {
      n->buffer->lpVtbl->Release(n->buffer);
    }
  }

  // NOTE: release textures
  for (R_D3D11_Texture *n = r_d3d11_state->first_to_free_texture, *next = 0; n != 0; n = next) {
    next = n->next;
    if (n->srv != 0) {
      n->srv->lpVtbl->Release(n->srv);
    }
    if (n->tex != 0) {
      n->tex->lpVtbl->Release(n->tex);
    }
    sll_stack_push(r_d3d11_state->first_free_texture, n);
  }

  // NOTE: reset state
  arena_clear(r_d3d11_state->buffer_flush_arena);
  r_d3d11_state->first_buffer_to_flush = r_d3d11_state->last_buffer_to_flush = 0;
  r_d3d11_state->first_to_free_texture = 0;
}

// NOTE: flush

internal void
r_flush(WM_Window handle, R_Window equip_handle, R_Rect_Batch_Group_List *groups) {
  R_D3D11_Window *window = r_d3d11_window_from_handle(equip_handle);
  ID3D11DeviceContext1 *d_ctx = r_d3d11_state->device_ctx;

  ID3D11VertexShader *vshader = r_d3d11_state->vshaders[R_D3D11_Vertex_Shader_Type_RECT];
  ID3D11InputLayout  *ilayout = r_d3d11_state->ilayouts[R_D3D11_Vertex_Shader_Type_RECT];
  ID3D11PixelShader  *pshader = r_d3d11_state->pshaders[R_D3D11_Pixel_Shader_Type_RECT];
  ID3D11Buffer       *uniform = r_d3d11_state->uniforms[R_D3D11_Uniform_Type_RECT];

  Vector2 resolution = window->last_resolution;
  u64 inst_size = sizeof(R_Rect);

  for (R_Rect_Batch_Group_Node *group_n = groups->first; group_n != 0; group_n = group_n->next) {
    R_Rect_Batch_List *batches = &group_n->batches;

    // NOTE: get & fill buffer
    ID3D11Buffer *buffer = r_d3d11_instance_buffer_from_size(batches->total_count*inst_size);
    {
      D3D11_MAPPED_SUBRESOURCE sub_rsrc = {0};
      d_ctx->lpVtbl->Map(d_ctx, (ID3D11Resource *)buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub_rsrc);
      u8 *dst_ptr = (u8 *)sub_rsrc.pData;
      u64 off = 0;
      for (R_Rect_Batch_Node *batch_n = batches->first; batch_n != 0; batch_n = batch_n->next) {
        u64 batch_size = batch_n->v.count*inst_size;
        memory_copy(dst_ptr+off, (u8 *)batch_n->v.v, batch_size);
        off += batch_size;
      }
      d_ctx->lpVtbl->Unmap(d_ctx, (ID3D11Resource *)buffer, 0);
    }

    // NOTE: get texture
    R_Texture texture_handle = group_n->params.texture;
    if (r_texture_match(texture_handle, r_texture_zero())) {
      texture_handle = r_d3d11_state->backup_texture;
    }
    R_D3D11_Texture *texture = r_d3d11_texture_from_handle(texture_handle);

    // NOTE: upload uniforms
    R_D3D11_Uniforms_Rect uniforms = {0};
    {
      uniforms.viewport_size_px = resolution;
      uniforms.texture_size_px = texture->size;
    }
    {
      D3D11_MAPPED_SUBRESOURCE sub_rsrc = {0};
      d_ctx->lpVtbl->Map(d_ctx, (ID3D11Resource *)uniform, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub_rsrc);
      memory_copy((u8 *)sub_rsrc.pData, &uniforms, sizeof(uniforms));
      d_ctx->lpVtbl->Unmap(d_ctx, (ID3D11Resource *)uniform, 0);
    }

    // NOTE: setup output merger
    d_ctx->lpVtbl->OMSetRenderTargets(d_ctx, 1, &window->stage_color_rtv, 0);
    d_ctx->lpVtbl->OMSetDepthStencilState(d_ctx, r_d3d11_state->noop_depth_stencil, 0);
    d_ctx->lpVtbl->OMSetBlendState(d_ctx, r_d3d11_state->main_blend_state, 0, 0xffffffff);

    // NOTE: setup rasterizer
    D3D11_VIEWPORT viewport = {0.0f, 0.0f, resolution.x, resolution.y, 0.0f, 1.0f};
    d_ctx->lpVtbl->RSSetViewports(d_ctx, 1, &viewport);
    d_ctx->lpVtbl->RSSetState(d_ctx, r_d3d11_state->main_rasterizer);

    // NOTE: setup input assembly
    u32 stride = (u32)inst_size;
    u32 offset = 0;
    d_ctx->lpVtbl->IASetPrimitiveTopology(d_ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    d_ctx->lpVtbl->IASetInputLayout(d_ctx, ilayout);
    d_ctx->lpVtbl->IASetVertexBuffers(d_ctx, 0, 1, &buffer, &stride, &offset);

    // NOTE: setup shaders
    d_ctx->lpVtbl->VSSetShader(d_ctx, vshader, 0, 0);
    d_ctx->lpVtbl->VSSetConstantBuffers(d_ctx, 0, 1, &uniform);
    d_ctx->lpVtbl->PSSetShader(d_ctx, pshader, 0, 0);
    d_ctx->lpVtbl->PSSetShaderResources(d_ctx, 0, 1, &texture->srv);
    d_ctx->lpVtbl->PSSetSamplers(d_ctx, 0, 1, &r_d3d11_state->nearest_sampler);

    // NOTE: setup scissor rect
    D3D11_RECT rect = {0};
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = (LONG)resolution.x;
    rect.bottom = (LONG)resolution.y;
    d_ctx->lpVtbl->RSSetScissorRects(d_ctx, 1, &rect);

    // NOTE: draw
    d_ctx->lpVtbl->DrawInstanced(d_ctx, 4, (UINT)batches->total_count, 0, 0);
  }
}
