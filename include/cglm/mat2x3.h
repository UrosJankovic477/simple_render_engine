/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

/*
 Macros:
   GLM_MAT2X3_ZERO_INIT
   GLM_MAT2X3_ZERO

 Functions:
   CGLM_INLINE void glm_mat2x3_copy(mat2x3 mat, mat2x3 dest);
   CGLM_INLINE void glm_mat2x3_zero(mat2x3 mat);
   CGLM_INLINE void glm_mat2x3_make(float * __restrict src, mat2x3 dest);
   CGLM_INLINE void glm_mat2x3_mul(mat2x3 m1, mat3x2 m2, mat2 dest);
   CGLM_INLINE void glm_mat2x3_mulv(mat2x3 m, vec3 v, vec2 dest);
   CGLM_INLINE void glm_mat2x3_transpose(mat2x3 m, mat3x2 dest);
   CGLM_INLINE void glm_mat2x3_scale(mat2x3 m, float s);
 */

#ifndef cglm_mat2x3_h
#define cglm_mat2x3_h

#include "common.h"

#define GLM_MAT2X3_ZERO_INIT {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}

/* for C only */
#define GLM_MAT2X3_ZERO GLM_MAT2X3_ZERO_INIT

/*!
 * @brief copy all members of [mat] to [dest]
 *
 * @param[in]  mat  source
 * @param[out] dest destination
 */
CGLM_INLINE
void
glm_mat2x3_copy(mat2x3 mat, mat2x3 dest) {
  dest[0][0] = mat[0][0];
  dest[0][1] = mat[0][1];
  dest[0][2] = mat[0][2];

  dest[1][0] = mat[1][0];
  dest[1][1] = mat[1][1];
  dest[1][2] = mat[1][2];
}

/*!
 * @brief make given matrix zero.
 *
 * @param[in, out]  mat  matrix
 */
CGLM_INLINE
void
glm_mat2x3_zero(mat2x3 mat) {
  CGLM_ALIGN_MAT mat2x3 t = GLM_MAT2X3_ZERO_INIT;
  glm_mat2x3_copy(t, mat);
}

/*!
 * @brief Create mat2x3 matrix from pointer
 *
 * @param[in]  src  pointer to an array of floats
 * @param[out] dest matrix
 */
CGLM_INLINE
void
glm_mat2x3_make(float * __restrict src, mat2x3 dest) {
  dest[0][0] = src[0];
  dest[0][1] = src[1];
  dest[0][2] = src[2];

  dest[1][0] = src[3];
  dest[1][1] = src[4];
  dest[1][2] = src[5];
}

/*!
 * @brief multiply m1 and m2 to dest
 *
 * m1, m2 and dest matrices can be same matrix, it is possible to write this:
 *
 * @code
 * glm_mat2x3_mul(m, m, m);
 * @endcode
 *
 * @param[in]  m1   left matrix
 * @param[in]  m2   right matrix
 * @param[out] dest destination matrix
 */
CGLM_INLINE
void
glm_mat2x3_mul(mat2x3 m1, mat3x2 m2, mat2 dest) {
  float a00 = m1[0][0], a01 = m1[0][1], a02 = m1[0][2],
        a10 = m1[1][0], a11 = m1[1][1], a12 = m1[1][2],

        b00 = m2[0][0], b01 = m2[0][1],
        b10 = m2[1][0], b11 = m2[1][1],
        b20 = m2[2][0], b21 = m2[2][1];

  dest[0][0] = a00 * b00 + a01 * b10 + a02 * b20;
  dest[0][1] = a00 * b01 + a01 * b11 + a02 * b21;

  dest[1][0] = a10 * b00 + a11 * b10 + a12 * b20;
  dest[1][1] = a10 * b01 + a11 * b11 + a12 * b21;
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
glm_mat2x3_mulv(mat2x3 m, vec3 v, vec2 dest) {
  float v0 = v[0], v1 = v[1], v2 = v[2];

  dest[0] = m[0][0] * v0 + m[0][1] * v1 + m[0][2] * v2;
  dest[1] = m[1][0] * v0 + m[1][1] * v1 + m[1][2] * v2;
}

/*!
 * @brief transpose matrix and store in dest
 *
 * @param[in]  m     matrix
 * @param[out] dest  result
 */
CGLM_INLINE
void
glm_mat2x3_transpose(mat2x3 m, mat3x2 dest) {
  dest[0][0] = m[0][0];  dest[0][1] = m[1][0];
  dest[1][0] = m[0][1];  dest[1][1] = m[1][1];
  dest[2][0] = m[0][2];  dest[2][1] = m[1][2];
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
glm_mat2x3_scale(mat2x3 m, float s) {
  m[0][0] *= s;  m[0][1] *= s;  m[0][2] *= s;
  m[1][0] *= s;  m[1][1] *= s;  m[1][2] *= s;
}

#endif
