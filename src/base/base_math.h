#ifndef BASE_MATH_H
#define BASE_MATH_H

////////////////////////////////
// NOTE: Vector Types

typedef union Vector2 Vector2;
union Vector2 {
  struct {f32 x, y;};
  f32 v[2];
};

typedef union Vector3 Vector3;
union Vector3 {
  struct {f32 x, y, z;};
  struct {Vector2 xy; f32 _z0;};
  struct {f32 _x0; Vector2 yz;};
  f32 v[3];
};

typedef union Vector4 Vector4;
union Vector4 {
  struct {f32 x, y, z, w;};
  struct {Vector2 xy, zw;};
  struct {Vector3 xyz; f32 _w0;};
  struct {Vector3 _x0; f32 yzw;};
  f32 v[4];
};

////////////////////////////////
// NOTE: Matrix Types

typedef struct Matrix4x4 Matrix4x4;
struct Matrix4x4 {
  f32 v[4][4];
};

////////////////////////////////
// NOTE: Range Types

typedef union Range2 Range2;
union Range2 {
  struct {Vector2 min, max;};
  struct {Vector2 p0, p1;};
  struct {f32 x0, y0, x1, y1;};
  Vector2 v[2];
};

////////////////////////////////
// NOTE: Scalar Math Ops

#define abs_s64(x) (s64)llabs(x)

#define sqrt_f32(x)   sqrtf(x)
#define cbrt_f32(x)   cbrtf(x)
#define mod_f32(a, b) fmodf((a), (b))
#define pow_f32(b, e) powf((b), (e))
#define ceil_f32(x)   ceilf(x)
#define floor_f32(x)  floorf(x)
#define round_f32(x)  roundf(x)
#define abs_f32(x)    fabsf(x)
#define sin_f32(x)    sinf(x)
#define cos_f32(x)    cosf(x)
#define tan_f32(x)    tanf(x)

#define sqrt_f64(x)   sqrt(x)
#define cbrt_f64(x)   cbrt(x)
#define mod_f64(a, b) fmod((a), (b))
#define pow_f64(b, e) pow((b), (e))
#define ceil_f64(x)   ceil(x)
#define floor_f64(x)  floor(x)
#define round_f64(x)  round(x)
#define abs_f64(x)    fabs(x)
#define sin_f64(x)    sin(x)
#define cos_f64(x)    cos(x)
#define tan_f64(x)    tan(x)

internal f32 lerp_f32(f32 a, f32 b, f32 t);
internal f32 lerpc_f32(f32 a, f32 b, f32 t);

////////////////////////////////
// NOTE: Vector Ops

// NOTE: vector2
internal Vector2 vector2_zero(void);
internal Vector2 vector2_fill(f32 n);
internal Vector2 vector2_make(f32 x, f32 y);
internal Vector2 vector2_from_array(f32 *v);
internal Vector2 vector2_add(Vector2 a, Vector2 b);
internal Vector2 vector2_sub(Vector2 a, Vector2 b);
internal Vector2 vector2_mul(Vector2 a, Vector2 b);
internal Vector2 vector2_div(Vector2 a, Vector2 b);
internal Vector2 vector2_adds(Vector2 v, f32 s);
internal Vector2 vector2_subs(Vector2 v, f32 s);
internal Vector2 vector2_muls(Vector2 v, f32 s);
internal Vector2 vector2_divs(Vector2 v, f32 s);
internal Vector2 vector2_negate(Vector2 v);
internal Vector2 vector2_normalize(Vector2 v);
internal Vector2 vector2_rotate(Vector2 v, f32 angle);
internal Vector2 vector2_lerp(Vector2 a, Vector2 b, f32 t);
internal Vector2 vector2_lerpc(Vector2 a, Vector2 b, f32 t);
internal f32 vector2_dot(Vector2 a, Vector2 b);
internal f32 vector2_cross(Vector2 a, Vector2 b);
internal f32 vector2_norm2(Vector2 v);
internal f32 vector2_norm(Vector2 v);

// NOTE: vector3
internal Vector3 vector3_zero(void);
internal Vector3 vector3_fill(f32 n);
internal Vector3 vector3_make(f32 x, f32 y, f32 z);
internal Vector3 vector3_from_array(f32 *v);
internal Vector3 vector3_from_2(Vector2 v, f32 z);
internal Vector3 vector3_add(Vector3 a, Vector3 b);
internal Vector3 vector3_sub(Vector3 a, Vector3 b);
internal Vector3 vector3_mul(Vector3 a, Vector3 b);
internal Vector3 vector3_div(Vector3 a, Vector3 b);
internal Vector3 vector3_adds(Vector3 v, f32 s);
internal Vector3 vector3_subs(Vector3 v, f32 s);
internal Vector3 vector3_muls(Vector3 v, f32 s);
internal Vector3 vector3_divs(Vector3 v, f32 s);
internal Vector3 vector3_negate(Vector3 v);
internal Vector3 vector3_normalize(Vector3 v);
internal Vector3 vector3_lerp(Vector3 a, Vector3 b, f32 t);
internal Vector3 vector3_lerpc(Vector3 a, Vector3 b, f32 t);
internal Vector3 vector3_cross(Vector3 a, Vector3 b);
internal Vector3 vector3_crossn(Vector3 a, Vector3 b);
internal f32 vector3_dot(Vector3 a, Vector3 b);
internal f32 vector3_norm2(Vector3 v);
internal f32 vector3_norm(Vector3 v);

// NOTE: vector4
internal Vector4 vector4_zero(void);
internal Vector4 vector4_fill(f32 n);
internal Vector4 vector4_make(f32 x, f32 y, f32 z, f32 w);
internal Vector4 vector4_from_array(f32 *v);
internal Vector4 vector4_from_3(Vector3 v, f32 w);
internal Vector4 vector4_add(Vector4 a, Vector4 b);
internal Vector4 vector4_sub(Vector4 a, Vector4 b);
internal Vector4 vector4_mul(Vector4 a, Vector4 b);
internal Vector4 vector4_div(Vector4 a, Vector4 b);
internal Vector4 vector4_adds(Vector4 v, f32 s);
internal Vector4 vector4_subs(Vector4 v, f32 s);
internal Vector4 vector4_muls(Vector4 v, f32 s);
internal Vector4 vector4_divs(Vector4 v, f32 s);
internal Vector4 vector4_negate(Vector4 v);
internal Vector4 vector4_normalize(Vector4 v);
internal Vector4 vector4_lerp(Vector4 a, Vector4 b, f32 t);
internal Vector4 vector4_lerpc(Vector4 a, Vector4 b, f32 t);
internal f32 vector4_dot(Vector4 a, Vector4 b);
internal f32 vector4_norm2(Vector4 v);
internal f32 vector4_norm(Vector4 v);

////////////////////////////////
// NOTE: Matrix Ops

// NOTE: all matrix ops treat the memory as row major.
internal Matrix4x4 matrix4x4_identity(void);
internal Matrix4x4 matrix4x4_translate(Vector3 v);
internal Matrix4x4 matrix4x4_scale(Vector3 v);
internal Matrix4x4 matrix4x4_rotate_x(f32 angle);
internal Matrix4x4 matrix4x4_rotate_y(f32 angle);
internal Matrix4x4 matrix4x4_rotate_z(f32 angle);
internal Matrix4x4 matrix4x4_ortho_rh_no(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);
internal Matrix4x4 matrix4x4_persp_rh_no(f32 fov_y, f32 aspect_w_over_h, f32 near_z, f32 far_z);
internal Matrix4x4 matrix4x4_lookat_rh(Vector3 eye, Vector3 target, Vector3 up);
internal Matrix4x4 matrix4x4_look_rh(Vector3 eye, Vector3 dir, Vector3 up);
internal Matrix4x4 matrix4x4_mul(Matrix4x4 a, Matrix4x4 b);
internal Vector4 matrix4x4_mul4(Matrix4x4 m, Vector4 v);
internal Vector3 matrix4x4_mul3(Matrix4x4 m, Vector3 v, f32 w);

////////////////////////////////
// NOTE: Range Ops

internal Range2 range2_zero(void);
internal Range2 range2_make(f32 x0, f32 y0, f32 x1, f32 y1);
internal Range2 range2_min_max(Vector2 min, Vector2 max);
internal Vector2 range2_dim(Range2 r);
internal b32 range2_contains(Range2 r, Vector2 v);

#endif // BASE_MATH_H
