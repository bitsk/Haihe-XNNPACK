// Auto-generated file. Do not edit!
//   Template: src/f16-velu/avx2-rr1-p3.c.in
//   Generator: tools/xngen
//
// Copyright 2022 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include <immintrin.h>

#include <xnnpack/common.h>
#include <xnnpack/vunary.h>


void xnn_f16_velu_ukernel__avx2_rr1_p3_x8(
    size_t n,
    const void* input,
    void* output,
    const union xnn_f16_elu_params params[restrict XNN_MIN_ELEMENTS(1)])
{
  assert(n % sizeof(uint16_t) == 0);

  const __m256 vprescale = _mm256_load_ps(params->avx2_rr1_p3.prescale);
  const __m256 vsat_cutoff = _mm256_load_ps(params->avx2_rr1_p3.sat_cutoff);
  const __m256 vmagic_bias = _mm256_load_ps(params->avx2_rr1_p3.magic_bias);
  const __m256 vlog2e = _mm256_load_ps(params->avx2_rr1_p3.log2e);
  const __m256 vminus_ln2 = _mm256_load_ps(params->avx2_rr1_p3.minus_ln2);
  const __m256 vc3 = _mm256_load_ps(params->avx2_rr1_p3.c3);
  const __m256 vc2 = _mm256_load_ps(params->avx2_rr1_p3.c2);
  const __m256 vc1 = _mm256_load_ps(params->avx2_rr1_p3.c1);
  const __m256 valpha = _mm256_load_ps(params->avx2_rr1_p3.alpha);
  const __m256 vbeta = _mm256_load_ps(params->avx2_rr1_p3.beta);

  const uint16_t* i = (const uint16_t*) input;
  uint16_t* o = (uint16_t*) output;
  for (; n >= 8 * sizeof(uint16_t); n -= 8 * sizeof(uint16_t)) {
    __m256 vx = _mm256_cvtph_ps(_mm_loadu_si128((const __m128i*) i));
    i += 8;

    const __m256 vz = _mm256_max_ps(vsat_cutoff, _mm256_mul_ps(vx, vprescale));

    __m256 vn = _mm256_fmadd_ps(vz, vlog2e, vmagic_bias);
    __m256 vs = _mm256_castsi256_ps(_mm256_slli_epi32(_mm256_castps_si256(vn), 23));
    vn = _mm256_sub_ps(vn, vmagic_bias);
    __m256 vt = _mm256_fmadd_ps(vn, vminus_ln2, vz);

    __m256 vp = _mm256_fmadd_ps(vc3, vt, vc2);
    vp = _mm256_fmadd_ps(vp, vt, vc1);
    vt = _mm256_mul_ps(vt, valpha);
    vt = _mm256_mul_ps(vt, vs);
    vs = _mm256_fmsub_ps(vs, valpha, valpha);
    const __m256 ve = _mm256_fmadd_ps(vp, vt, vs);
    vx = _mm256_mul_ps(vx, vbeta);
    const __m256 vy = _mm256_blendv_ps(vx, ve, vx);

    _mm_storeu_si128((__m128i*) o, _mm256_cvtps_ph(vy, _MM_FROUND_NO_EXC));
    o += 8;
  }
  if XNN_UNLIKELY(n != 0) {
    assert(n >= 1 * sizeof(uint16_t));
    assert(n <= 7 * sizeof(uint16_t));
    __m256 vx = _mm256_cvtph_ps(_mm_loadu_si128((const __m128i*) i));

    const __m256 vz = _mm256_max_ps(vsat_cutoff, _mm256_mul_ps(vx, vprescale));

    __m256 vn = _mm256_fmadd_ps(vz, vlog2e, vmagic_bias);
    __m256 vs = _mm256_castsi256_ps(_mm256_slli_epi32(_mm256_castps_si256(vn), 23));
    vn = _mm256_sub_ps(vn, vmagic_bias);
    __m256 vt = _mm256_fmadd_ps(vn, vminus_ln2, vz);

    __m256 vp = _mm256_fmadd_ps(vc3, vt, vc2);
    vp = _mm256_fmadd_ps(vp, vt, vc1);
    vt = _mm256_mul_ps(vt, valpha);
    vt = _mm256_mul_ps(vt, vs);
    vs = _mm256_fmsub_ps(vs, valpha, valpha);
    const __m256 ve = _mm256_fmadd_ps(vp, vt, vs);
    vx = _mm256_mul_ps(vx, vbeta);
    const __m256 vy = _mm256_blendv_ps(vx, ve, vx);

    __m128i vh = _mm256_cvtps_ph(vy, _MM_FROUND_NO_EXC);
    if (n & (4 * sizeof(uint16_t))) {
      _mm_storel_epi64((__m128i*) o, vh);
      vh = _mm_unpackhi_epi64(vh, vh);
      o += 4;
    }
    if (n & (2 * sizeof(uint16_t))) {
      _mm_storeu_si32(o, vh);
      vh = _mm_srli_epi64(vh, 32);
      o += 2;
    }
    if (n & (1 * sizeof(uint16_t))) {
      *o = (uint16_t) _mm_extract_epi16(vh, 0);
    }
  }
}