/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

/*
 Macros:
   GLM_MAT2X4_ZERO_INIT
   GLM_MAT2X4_ZERO

 Functions:
   CGLM_INLINE void glm_mat2x4_copy(mat2x4 mat, mat2x4 dest);
   CGLM_INLINE void glm_mat2x4_zero(mat2x4 mat);
   CGLM_INLINE void glm_mat2x4_make(float * __restrict src, mat2x4 dest);
   CGLM_INLINE void glm_mat2x4_mul(mat2x4 m1, mat4x2 m2, mat2 dest);
   CGLM_INLINE void glm_mat2x4_mulv(mat2x4 m, vec4 v, vec2 dest);
   CGLM_INLINE void glm_mat2x4_transpose(mat2x4 m, mat4x2 dest);
   CGLM_INLINE void glm_mat2x4_scale(mat2x4 m, float s);
 */

#ifndef cglm_mat2x4_h
#define cglm_mat2x4_h

#include "common.h"
#include "vec4.h"

#define GLM_MAT2X4_ZERO_INIT {{0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}

/* for C only */
#define GLM_MAT2X4_ZERO GLM_MAT2X4_ZERO_INIT

/*!
 * @brief copy all members of [mat] to [dest]
 *
 * @param[in]  mat  source
 * @param[out] dest destination
 */
CGLM_INLINE
void
glm_mat2x4_copy(mat2x4 mat, mat2x4 dest) {
  glm_vec4_ucopy(mat[0], dest[0]);
  glm_vec4_ucopy(mat[1], dest[1]);
}

/*!
 * @brief make given matrix zero.
 *
 * @param[in, out]  mat  matrix
 */
CGLM_INLINE
void
glm_mat2x4_zero(mat2x4 mat) {
  CGLM_ALIGN_MAT mat2x4 t = GLM_MAT2X4_ZERO_INIT;
  glm_mat2x4_copy(t, mat);
}

/*!
 * @brief Create mat2x4 matrix from pointer
 *
 * @param[in]  src  pointer to an array of floats
 * @param[out] dest matrix
 */
CGLM_INLINE
void
glm_mat2x4_make(float * __restrict src, mat2x4 dest) {
  dest[0][0] = src[0];
  dest[0][1] = src[1];
  dest[0][2] = src[2];
  dest[0][3] = src[3];

  dest[1][0] = src[4];
  dest[1][1] = src[5];
  dest[1][2] = src[6];
  dest[1][3] = src[7];
}

/*!
 * @brief multiply m1 and m2 to dest
 *
 * m1, m2 and dest matrices can be same matrix, it is possible to write this:
 *
 * @code
 * glm_mat2x4_mul(m, m, m);
 * @endcode
 *
 * @param[in]  m1   left matrix
 * @param[in]  m2   right matrix
 * @param[out] dest destination matrix
 */
CGLM_INLINE
void
glm_mat2x4_mul(mat2x4 m1, mat4x2 m2, mat2 dest) {
  float a00 = m1[0][0], a01 = m1[0][1], a02 = m1[0][2], a03 = m1[0][3],
        a10 = m1[1][0], a11 = m1[1][1], a12 = m1[1][2], a13 = m1[1][3],

        b00 = m2[0][0], b01 = m2[0][1],
        b10 = m2[1][0], b11 = m2[1][1],
        b20 = m2[2][0], b21 = m2[2][1],
        b30 = m2[3][0], b31 = m2[3][1];

  dest[0][0] = a00 * b00 + a01 * b10 + a02 * b20 + a03 * b30;
  dest[0][1] = a00 * b01 + a01 * b11 + a02 * b21 + a03 * b31;

  dest[1][0] = a10 * b00 + a11 * b10 + a12 * b20 + a13 * b30;
  dest[1][1] = a10 * b01 + a11 * b11 + a12 * b21 + a13 * b31;
}

/*!
 * @brief multiply matrix with column vector and store in dest vector
 *
 * @param[in]  m    matrix (left)
 * @param[in]  v    vector (right, column vector)
 * @param[out] dest result vector
 */
CGLM_INLINE
void
glm_mat2x4_mulv(mat2x4 m, vec4 v, vec2 dest) {
  float v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];

  dest[0] = m[0][0] * v0 + m[0][1] * v1 + m[0][2] * v2 + m[0][3] * v3;
  dest[1] = m[1][0] * v0 + m[1][1] * v1 + m[1][2] * v2 + m[1][3] * v3;
}

/*!
 * @brief transpose matrix and store in dest
 *
 * @param[in]  m     matrix
 * @param[out] dest  result
 */
CGLM_INLINE
void
glm_mat2x4_transpose(mat2x4 m, mat4x2 dest) {
  dest[0][0] = m[0][0];  dest[0][1] = m[1][0];
  dest[1][0] = m[0][1];  dest[1][1] = m[1][1];
  dest[2][0] = m[0][2];  dest[2][1] = m[1][2];
  dest[3][0] = m[0][3];  dest[3][1] = m[1][3];
}

/*!
 * @brief scale (multiply with scalar) matrix
 *
 * multiply matrix with scalar
 *
 * @param[in, out] m matrix
 * @param[in]    s scalar
 */
CGLM_INLINE
void
glm_mat2x4_scale(mat2x4 m, float s) {
  m[0][0] *= s;  m[0][1] *= s;  m[0][2] *= s;  m[0][3] *= s;
  m[1][0] *= s;  m[1][1] *= s;  m[1][2] *= s;  m[1][3] *= s;
}

#endif
