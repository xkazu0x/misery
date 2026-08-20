////////////////////////////////
// NOTE: Scalar Math Ops

internal f32
lerp_f32(f32 a, f32 b, f32 t) {
  f32 result = a + t*(b - a);
  return(result);
}

internal f32
lerpc_f32(f32 a, f32 b, f32 t) {
  f32 result = lerp_f32(a, b, clamp(t, 0.0f, 1.0f));
  return(result);
}

////////////////////////////////
// NOTE: Vector Ops

// NOTE: vector2

internal Vector2
vector2_zero(void) {
  Vector2 result = {0};
  return(result);
}

internal Vector2
vector2_fill(f32 n) {
  Vector2 result = {n, n};
  return(result);
}

internal Vector2
vector2_make(f32 x, f32 y) {
  Vector2 result = {x, y};
  return(result);
}

internal Vector2
vector2_from_array(f32 *v) {
  Vector2 result = {v[0], v[1]};
  return(result);
}

internal Vector2
vector2_add(Vector2 a, Vector2 b) {
  Vector2 result = {a.x + b.x, a.y + b.y};
  return(result);
}

internal Vector2
vector2_sub(Vector2 a, Vector2 b) {
  Vector2 result = {a.x - b.x, a.y - b.y};
  return(result);
}

internal Vector2
vector2_mul(Vector2 a, Vector2 b) {
  Vector2 result = {a.x*b.x, a.y*b.y};
  return(result);
}

internal Vector2
vector2_div(Vector2 a, Vector2 b) {
  Vector2 result = {a.x/b.x, a.y/b.y};
  return(result);
}

internal Vector2
vector2_adds(Vector2 v, f32 s) {
  Vector2 result = {v.x + s, v.y + s};
  return(result);
}

internal Vector2
vector2_subs(Vector2 v, f32 s) {
  Vector2 result = {v.x - s, v.y - s};
  return(result);
}

internal Vector2
vector2_muls(Vector2 v, f32 s) {
  Vector2 result = {v.x*s, v.y*s};
  return(result);
}

internal Vector2
vector2_divs(Vector2 v, f32 s) {
  Vector2 result = {v.x/s, v.y/s};
  return(result);
}

internal Vector2
vector2_negate(Vector2 v) {
  Vector2 result = {-v.x, -v.y};
  return(result);
}

internal Vector2 
vector2_normalize(Vector2 v) {
  Vector2 result = vector2_muls(v, (1.0f/vector2_norm(v)));
  return(result);
}

internal Vector2
vector2_rotate(Vector2 v, f32 angle) {
  Vector2 result;
  f32 c = cos_f32(angle);
  f32 s = sin_f32(angle);
  result.x = v.x*c - v.y*s;
  result.y = v.x*s + v.y*c;
  return(result);
}

internal Vector2
vector2_lerp(Vector2 a, Vector2 b, f32 t) {
  Vector2 v, s;
  s = vector2_fill(t);
  v = vector2_sub(b, a);
  v = vector2_mul(s, v);
  v = vector2_add(a, v);
  return(v);
}

internal Vector2
vector2_lerpc(Vector2 a, Vector2 b, f32 t) {
  Vector2 result = vector2_lerp(a, b, clamp(t, 0.0f, 1.0f));
  return(result);
}

internal f32
vector2_dot(Vector2 a, Vector2 b) {
  f32 result = a.x*b.x + a.y*b.y;
  return(result);
}

internal f32
vector2_cross(Vector2 a, Vector2 b) {
  f32 result = a.x*b.y - a.y*b.x;
  return(result);
}

internal f32
vector2_norm2(Vector2 v) {
  f32 result = vector2_dot(v, v);
  return(result);
}

internal f32 
vector2_norm(Vector2 v) {
  f32 result = sqrt_f32(vector2_norm2(v));
  return(result);
}

// NOTE: vector3

internal Vector3
vector3_zero(void) {
  Vector3 result = {0};
  return(result);
}

internal Vector3
vector3_fill(f32 n) {
  Vector3 result = {n, n, n};
  return(result);
}

internal Vector3
vector3_make(f32 x, f32 y, f32 z) {
  Vector3 result = {x, y, z};
  return(result);
}

internal Vector3
vector3_from_array(f32 *v) {
  Vector3 result = {v[0], v[1], v[2]};
  return(result);
}

internal Vector3
vector3_from_2(Vector2 v, f32 z) {
  Vector3 result = {v.x, v.y, z};
  return(result);
}

internal Vector3
vector3_add(Vector3 a, Vector3 b) {
  Vector3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
  return(result);
}

internal Vector3
vector3_sub(Vector3 a, Vector3 b) {
  Vector3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
  return(result);
}

internal Vector3
vector3_mul(Vector3 a, Vector3 b) {
  Vector3 result = {a.x*b.x, a.y*b.y, a.z*b.z};
  return(result);
}

internal Vector3
vector3_div(Vector3 a, Vector3 b) {
  Vector3 result = {a.x/b.x, a.y/b.y, a.z/b.z};
  return(result);
}

internal Vector3
vector3_adds(Vector3 v, f32 s) {
  Vector3 result = {v.x + s, v.y + s, v.z + s};
  return(result);
}

internal Vector3
vector3_subs(Vector3 v, f32 s) {
  Vector3 result = {v.x - s, v.y - s, v.z - s};
  return(result);
}

internal Vector3
vector3_muls(Vector3 v, f32 s) {
  Vector3 result = {v.x*s, v.y*s, v.z*s};
  return(result);
}

internal Vector3
vector3_divs(Vector3 v, f32 s) {
  Vector3 result = {v.x/s, v.y/s, v.z/s};
  return(result);
}

internal Vector3
vector3_negate(Vector3 v) {
  Vector3 result = {-v.x, -v.y, -v.z};
  return(result);
}

internal Vector3 
vector3_normalize(Vector3 v) {
  Vector3 result = vector3_muls(v, (1.0f/vector3_norm(v)));
  return(result);
}

internal Vector3
vector3_lerp(Vector3 a, Vector3 b, f32 t) {
  Vector3 v, s;
  s = vector3_fill(t);
  v = vector3_sub(b, a);
  v = vector3_mul(s, v);
  v = vector3_add(a, v);
  return(v);
}

internal Vector3
vector3_lerpc(Vector3 a, Vector3 b, f32 t) {
  Vector3 result = vector3_lerp(a, b, clamp(t, 0.0f, 1.0f));
  return(result);
}

internal Vector3
vector3_cross(Vector3 a, Vector3 b) {
  Vector3 result = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
  return(result);
}

internal Vector3
vector3_crossn(Vector3 a, Vector3 b) {
  Vector3 result = vector3_normalize(vector3_cross(a, b));
  return(result);
}

internal f32
vector3_dot(Vector3 a, Vector3 b) {
  f32 result = a.x*b.x + a.y*b.y + a.z*b.z;
  return(result);
}

internal f32
vector3_norm2(Vector3 v) {
  f32 result = vector3_dot(v, v);
  return(result);
}

internal f32 
vector3_norm(Vector3 v) {
  f32 result = sqrt_f32(vector3_norm2(v));
  return(result);
}

// NOTE: vector4

internal Vector4
vector4_zero(void) {
  Vector4 result = {0};
  return(result);
}

internal Vector4
vector4_fill(f32 n) {
  Vector4 result = {n, n, n, n};
  return(result);
}

internal Vector4
vector4_make(f32 x, f32 y, f32 z, f32 w) {
  Vector4 result = {x, y, z, w};
  return(result);
}

internal Vector4
vector4_from_array(f32 *v) {
  Vector4 result = {v[0], v[1], v[2], v[3]};
  return(result);
}

internal Vector4
vector4_from_3(Vector3 v, f32 w) {
  Vector4 result = {v.x, v.y, v.z, w};
  return(result);
}

internal Vector4
vector4_add(Vector4 a, Vector4 b) {
  Vector4 result = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
  return(result);
}

internal Vector4
vector4_sub(Vector4 a, Vector4 b) {
  Vector4 result = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
  return(result);
}

internal Vector4
vector4_mul(Vector4 a, Vector4 b) {
  Vector4 result = {a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w};
  return(result);
}

internal Vector4
vector4_div(Vector4 a, Vector4 b) {
  Vector4 result = {a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w};
  return(result);
}

internal Vector4
vector4_adds(Vector4 v, f32 s) {
  Vector4 result = {v.x + s, v.y + s, v.z + s, v.w + s};
  return(result);
}

internal Vector4
vector4_subs(Vector4 v, f32 s) {
  Vector4 result = {v.x - s, v.y - s, v.z - s, v.w - s};
  return(result);
}

internal Vector4
vector4_muls(Vector4 v, f32 s) {
  Vector4 result = {v.x*s, v.y*s, v.z*s, v.w*s};
  return(result);
}

internal Vector4
vector4_divs(Vector4 v, f32 s) {
  Vector4 result = {v.x/s, v.y/s, v.z/s, v.w/s};
  return(result);
}

internal Vector4
vector4_negate(Vector4 v) {
  Vector4 result = {-v.x, -v.y, -v.z, -v.w};
  return(result);
}

internal Vector4 
vector4_normalize(Vector4 v) {
  Vector4 result = vector4_muls(v, (1.0f/vector4_norm(v)));
  return(result);
}

internal Vector4
vector4_lerp(Vector4 a, Vector4 b, f32 t) {
  Vector4 v, s;
  s = vector4_fill(t);
  v = vector4_sub(b, a);
  v = vector4_mul(s, v);
  v = vector4_add(a, v);
  return(v);
}

internal Vector4
vector4_lerpc(Vector4 a, Vector4 b, f32 t) {
  Vector4 result = vector4_lerp(a, b, clamp(t, 0.0f, 1.0f));
  return(result);
}

internal f32
vector4_dot(Vector4 a, Vector4 b) {
  f32 result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
  return(result);
}

internal f32
vector4_norm2(Vector4 v) {
  f32 result = vector4_dot(v, v);
  return(result);
}

internal f32 
vector4_norm(Vector4 v) {
  f32 result = sqrt_f32(vector4_norm2(v));
  return(result);
}

////////////////////////////////
// NOTE: Matrix Ops

internal Matrix4x4
matrix4x4_identity(void) {
  // | 1  0  0  0 |
  // | 0  1  0  0 |
  // | 0  0  1  0 |
  // | 0  0  0  1 |
  Matrix4x4 result = {0};
  result.v[0][0] = 1.0f;
  result.v[1][1] = 1.0f;
  result.v[2][2] = 1.0f;
  result.v[3][3] = 1.0f;
  return(result);
}

internal Matrix4x4
matrix4x4_translate(Vector3 v) {
  // | 1  0  0  x |
  // | 0  1  0  y |
  // | 0  0  1  z |
  // | 0  0  0  1 |
  Matrix4x4 result = matrix4x4_identity();
  result.v[0][3] = v.x;
  result.v[1][3] = v.y;
  result.v[2][3] = v.z;
  return(result);
}

internal Matrix4x4
matrix4x4_scale(Vector3 v) {
  // | x  0  0  0 |
  // | 0  y  0  0 |
  // | 0  0  z  0 |
  // | 0  0  0  1 |
  Matrix4x4 result = matrix4x4_identity();
  result.v[0][0] = v.x;
  result.v[1][1] = v.y;
  result.v[2][2] = v.z;
  return(result);
}

internal Matrix4x4
matrix4x4_rotate_x(f32 angle) {
  // | 1  0  0  0 |
  // | 0  c -s  0 |
  // | 0  s  c  0 |
  // | 0  0  0  1 |
  Matrix4x4 result = matrix4x4_identity();
  f32 c = cos_f32(angle);
  f32 s = sin_f32(angle);
  result.v[1][1] = c;
  result.v[1][2] =-s;
  result.v[2][1] = s;
  result.v[2][2] = c;
  return(result);
}

internal Matrix4x4
matrix4x4_rotate_y(f32 angle) {
  // | c  0  s  0 |
  // | 0  1  0  0 |
  // |-s  0  c  0 |
  // | 0  0  0  1 |
  Matrix4x4 result = matrix4x4_identity();
  f32 c = cos_f32(angle);
  f32 s = sin_f32(angle);
  result.v[0][0] = c;
  result.v[0][2] = s;
  result.v[2][0] =-s;
  result.v[2][2] = c;
  return(result);
}

internal Matrix4x4
matrix4x4_rotate_z(f32 angle) {
  // | c -s  0  0 |
  // | s  c  0  0 |
  // | 0  0  1  0 |
  // | 0  0  0  1 |
  Matrix4x4 result = matrix4x4_identity();
  f32 c = cos_f32(angle);
  f32 s = sin_f32(angle);
  result.v[0][0] = c;
  result.v[0][1] =-s;
  result.v[1][0] = s;
  result.v[1][1] = c;
  return(result);
}

internal Matrix4x4
matrix4x4_ortho_rh_no(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z) {
  // | 2*rl     0     0 -(r+l)*rl |
  // |    0  2*tb     0 -(t+b)*tb |
  // |    0     0  2*fn  (f+n)*fn |
  // |    0     0     0         0 |
  Matrix4x4 result = matrix4x4_identity();
  f32 rl = 1.0f/(right - left);
  f32 tb = 1.0f/(top - bottom);
  f32 fn =-1.0f/(far_z - near_z);
  result.v[0][0] = 2.0f*rl;
  result.v[1][1] = 2.0f*tb;
  result.v[2][2] = 2.0f*fn;
  result.v[0][3] =-(right + left)*rl;
  result.v[1][3] =-(top + bottom)*tb;
  result.v[2][3] = (far_z + near_z)*fn;
  return(result);
}

internal Matrix4x4
matrix4x4_persp_rh_no(f32 fov_y, f32 aspect_w_over_h, f32 near_z, f32 far_z) {
  // | f*a  0         0         0 |
  // |   0  f         0         0 |
  // |   0  0  (n+f)*fn  2*n*f*fn |
  // |   0  0        -1         0 |
  Matrix4x4 result = matrix4x4_identity();
  f32 f  = 1.0f/tan_f32(fov_y*0.5f);
  f32 fn = 1.0f/(near_z - far_z);
  result.v[0][0] = f/aspect_w_over_h;
  result.v[1][1] = f;
  result.v[2][2] = (near_z + far_z)*fn;
  result.v[2][3] = 2.0f*near_z*far_z*fn;
  result.v[3][2] =-1.0f;
  result.v[3][3] = 0.0f;
  return(result);
}

internal Matrix4x4
matrix4x4_lookat_rh(Vector3 eye, Vector3 target, Vector3 up) {
  // | s.x  s.y  s.z -dot(s,eye) |
  // | u.x  u.y  u.z -dot(u,eye) |
  // |-f.x -f.y -f.z  dot(f,eye) |
  // |   0    0    0           1 |
  Matrix4x4 result;
  Vector3 f = vector3_normalize(vector3_sub(target, eye));
  Vector3 s = vector3_crossn(f, up);
  Vector3 u = vector3_cross(s, f);
  result.v[0][0] = s.x;
  result.v[0][1] = s.y;
  result.v[0][2] = s.z;
  result.v[0][3] = -vector3_dot(s, eye);
  result.v[1][0] = u.x;
  result.v[1][1] = u.y;
  result.v[1][2] = u.z;
  result.v[1][3] = -vector3_dot(u, eye);
  result.v[2][0] = -f.x;
  result.v[2][1] = -f.y;
  result.v[2][2] = -f.z;
  result.v[2][3] = vector3_dot(f, eye);
  result.v[3][0] = 0.0f;
  result.v[3][1] = 0.0f;
  result.v[3][2] = 0.0f;
  result.v[3][3] = 1.0f;
  return(result);
}

internal Matrix4x4
matrix4x4_look_rh(Vector3 eye, Vector3 dir, Vector3 up) {
  Vector3 target = vector3_add(eye, dir);
  Matrix4x4 result = matrix4x4_lookat_rh(eye, target, up);
  return(result);
}

internal Matrix4x4
matrix4x4_mul(Matrix4x4 a, Matrix4x4 b) {
  Matrix4x4 result = {0};
  for (u32 i = 0; i < 4; i += 1) {
    for (u32 j = 0; j < 4; j += 1) {
      for (u32 k = 0; k < 4; k += 1) {
        result.v[i][j] += a.v[i][k]*b.v[k][j];
      }
    }
  }
  return(result);
}

internal Vector4
matrix4x4_mul4(Matrix4x4 m, Vector4 v) {
  Vector4 result = {0};
  for (u32 i = 0; i < 4; i += 1) {
    for (u32 j = 0; j < 4; j += 1) {
      result.v[i] += m.v[i][j]*v.v[j];
    }
  }
  return(result);
}

internal Vector3
matrix4x4_mul3(Matrix4x4 m, Vector3 v, f32 w) {
  Vector3 result = matrix4x4_mul4(m, vector4_from_3(v, w)).xyz;
  return(result);
}

////////////////////////////////
// NOTE: Range Ops

internal Range2
range2_zero(void) {
  Range2 result = {0};
  return(result);
}

internal Range2
range2_make(f32 x0, f32 y0, f32 x1, f32 y1) {
  Range2 result = {x0, y0, x1, y1};
  return(result);
}

internal Range2
range2_min_max(Vector2 min, Vector2 max) {
  Range2 result = {min, max};
  return(result);
}

internal Vector2
range2_dim(Range2 r) {
  Vector2 result = {((r.max.x > r.min.x) ? (r.max.x - r.min.x) : 0), ((r.max.y > r.min.y) ? (r.max.y - r.min.y) : 0)};
  return(result);
}

internal b32
range2_contains(Range2 r, Vector2 v) {
  b32 result = (r.min.x <= v.x && v.x < r.max.x && r.min.y <= v.y && v.y < r.max.y);
  return(result);
}
