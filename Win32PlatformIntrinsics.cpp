#ifdef PLATFORM_WINDOWS

#include "PlatformIntrinsics.h"

#include "ZBaseTypes.h"
#include "ZAssert.h"
#include "PlatformDefines.h"
#include "CommonMath.h"

#include <cstring>

#ifdef HW_PLATFORM_X86

#include <immintrin.h>
#include <intrin.h>

#include "Common.h"

FORCE_INLINE void CPUIDSection00(int buffer[4]) {
  __cpuid(buffer, 0x00);

  // Note: GenuineIntel is not written as expected so we need to swap the last couple DWORDS
  ZSharp::Swap(buffer[3], buffer[2]);
  // [0] = EAX
  // [1] = EBX
  // [2] = ECX
  // [3] = EDX
}

FORCE_INLINE void CPUIDSection01(int buffer[4]) {
  __cpuid(buffer, 0x01);

  // [0] = EAX
  // [1] = EBX
  // [2] = ECX
  // [3] = EDX
}

FORCE_INLINE void CPUIDSection07(int buffer[4]) {
  __cpuid(buffer, 0x07);

  // [0] = EAX
  // [1] = EBX
  // [2] = ECX
  // [3] = EDX
}

FORCE_INLINE void CPUIDSectionBrand(int buffer[12]) {
  __cpuid(buffer, 0x80000002);
  __cpuid(buffer + 4, 0x80000003);
  __cpuid(buffer + 8, 0x80000004);

  // [0] = EAX
  // [1] = EBX
  // [2] = ECX
  // [3] = EDX
}

FORCE_INLINE __m128i Not128(const __m128i v) {
  return _mm_xor_si128(_mm_cmpeq_epi32(v, v), v);
}

FORCE_INLINE __m256i Not256(const __m256i v) {
  return _mm256_xor_si256(_mm256_cmpeq_epi32(v, v), v);
}

FORCE_INLINE __m128 Lerp128(__m128 x1, __m128 x2, __m128 t) {
  __m128 mulResult = _mm_mul_ps(t, x2);
  __m128 subResult = _mm_sub_ps(_mm_set_ps1(1.f), t);
  __m128 result = _mm_add_ps(mulResult, _mm_mul_ps(subResult, x1));
  return result;
}

FORCE_INLINE __m256 Lerp256(__m256 x1, __m256 x2, __m256 t) {
  __m256 subResult = _mm256_sub_ps(_mm256_set1_ps(1.f), t);
  __m256 result = _mm256_fmadd_ps(t, x2, _mm256_mul_ps(subResult, x1));
  return result;
}

FORCE_INLINE __m128 Cross128(__m128 a, __m128 b) {
  __m128 a0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(a), 0b11001001));
  __m128 b0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(b), 0b11010010));

  __m128 a0b = _mm_mul_ps(a0, b);
  __m128 a0b0 = _mm_mul_ps(a0, b0);

  __m128 b1 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(a0b), 0b11001001));

  return _mm_sub_ps(a0b0, b1);
}

namespace ZSharp {

// TODO: Add more as needed here.
RGBShaderFunc RGBShaderImpl = nullptr;
UVShaderFunc UVShaderImpl = nullptr;
CalculateAABBFunc CalculateAABBImpl = nullptr;
DrawDebugTextFunc DrawDebugTextImpl = nullptr;
DepthBufferVisualizeFunc DepthBufferVisualizeImpl = nullptr;
BlendBuffersFunc BlendBuffersImpl = nullptr;
BilinearScaleImageFunc BilinearScaleImageImpl = nullptr;
GenerateMipLevelFunc GenerateMipLevelImpl = nullptr;

bool PlatformSupportsSIMDLanes(SIMDLaneWidth width) {
  int bits[4]{};

  switch (width) {
#if 0
    case SIMDWidth::Four:
    {
      // Checks for SSE2.
      int* bits = CPUIDSection01();
      return bits[3] & (1 << 26);
    }
    break;
#endif
    case SIMDLaneWidth::Four: // SSE4
    {
      CPUIDSection01(bits);
      return bits[2] & 524288; // (1 << 19)
    }
    break;
    case SIMDLaneWidth::Eight: // AVX2
    {
      CPUIDSection07(bits);
      return bits[1] & 32; // (1 << 5)
    }
    break;
    case SIMDLaneWidth::Sixteen: // AVX512
    {
      CPUIDSection07(bits);
      return bits[1] & 65536; // (1 << 16)
    }
    break;
  }

  return false;
}

size_t PlatformAlignmentGranularity() {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Sixteen)) {
    return 64;
  }
  else if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    return 32;
  } 
  else if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    return 16;
  }
  else {
    ZAssert(false);
    return 0;
  }
}

String PlatformCPUVendor() {
  int buffer[5]{};
  CPUIDSection00(buffer);
  String vendor((const char*)(buffer + 1));
  return vendor;
}

String PlatformCPUBrand() {
  int buffer[12]{};
  CPUIDSectionBrand(buffer);
  String brand((const char*)buffer);
  return brand;
}

void Aligned_128Add(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128AddInPlace(float* a, const float* b) {
  _mm_store_ps(a, _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128Sub(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_sub_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128SubInPlace(float* a, const float* b) {
  _mm_store_ps(a, _mm_sub_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128MulByValue(const float* a, const float b, float* dest) {
  _mm_store_ps(dest, _mm_mul_ps(_mm_load_ps(a), _mm_set_ps1(b)));
}

void Aligned_128MulByValueInPlace(float* a, const float b) {
  _mm_store_ps(a, _mm_mul_ps(_mm_load_ps(a), _mm_set_ps1(b)));
}

float Aligned_128MulSum(const float* a, const float* b) {
  __m128 mulResult = _mm_mul_ps(_mm_load_ps(a), _mm_load_ps(b));
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  return _mm_cvtss_f32(mulResult);
}

void Aligned_Memset(void* __restrict dest, uint32 value, const size_t numBytes) {
  __stosd((unsigned long*)dest, value, numBytes / 4);
}

void Aligned_Memcpy(void* __restrict dest, const void* __restrict src, size_t numBytes) {
  __movsb((unsigned char*)dest, (const unsigned char*)src, numBytes);
}

void Unaligned_128Add(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void Unaligned_128Sub(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void Unaligned_128MulByValue(const float* a, const float b, float* dest) {
  _mm_storeu_ps(dest, _mm_mul_ps(_mm_loadu_ps(a), _mm_set_ps1(b)));
}

float Unaligned_128MulSum(const float* a, const float* b) {
  __m128 mulResult = _mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  return _mm_cvtss_f32(mulResult);
}

float Unaligned_Vec3Length(const float* a) {
  __m128 x = _mm_load_ss(a);
  __m128 y = _mm_load_ss(a + 1);
  __m128 z = _mm_load_ss(a + 2);

  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_add_ss(_mm_add_ss(_mm_mul_ss(x, x), _mm_mul_ss(y, y)), _mm_mul_ss(z, z))));
}

void Unaligned_Vec3Normalize(float* a) {
  __m128 x = _mm_load_ss(a);
  __m128 y = _mm_load_ss(a + 1);
  __m128 z = _mm_load_ss(a + 2);

  __m128 sqrt = _mm_sqrt_ss(_mm_add_ss(_mm_add_ss(_mm_mul_ss(x, x), _mm_mul_ss(y, y)), _mm_mul_ss(z, z)));
  _mm_store_ss(a, _mm_div_ss(x, sqrt));
  _mm_store_ss(a + 1, _mm_div_ss(y, sqrt));
  _mm_store_ss(a + 2, _mm_div_ss(z, sqrt));
}

float Unaligned_Vec4Length(const float* a) {
  __m128 x = _mm_load_ss(a);
  __m128 y = _mm_load_ss(a + 1);
  __m128 z = _mm_load_ss(a + 2);
  __m128 w = _mm_load_ss(a + 3);

  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_add_ss(_mm_add_ss(_mm_add_ss(_mm_mul_ss(x, x), _mm_mul_ss(y, y)), _mm_mul_ss(z, z)), _mm_mul_ss(w, w))));
}

void Unaligned_Vec4Normalize(float* a) {
  __m128 x = _mm_load_ss(a);
  __m128 y = _mm_load_ss(a + 1);
  __m128 z = _mm_load_ss(a + 2);
  __m128 w = _mm_load_ss(a + 3);

  __m128 sqrt = _mm_sqrt_ss(_mm_add_ss(_mm_add_ss(_mm_add_ss(_mm_mul_ss(x, x), _mm_mul_ss(y, y)), _mm_mul_ss(z, z)), _mm_mul_ss(w, w)));
  _mm_store_ss(a, _mm_div_ss(x, sqrt));
  _mm_store_ss(a + 1, _mm_div_ss(y, sqrt));
  _mm_store_ss(a + 2, _mm_div_ss(z, sqrt));
  _mm_store_ss(a + 3, _mm_div_ss(w, sqrt));
}

void Unaligned_Vec4Homogenize(float* a) {
  __m128 x = _mm_load_ss(a);
  __m128 y = _mm_load_ss(a + 1);
  __m128 z = _mm_load_ss(a + 2);
  __m128 w = _mm_load_ss(a + 3);
  _mm_store_ss(a, _mm_div_ss(x, w));
  _mm_store_ss(a + 1, _mm_div_ss(y, w));
  _mm_store_ss(a + 2, _mm_div_ss(z, w));
  _mm_store_ss(a + 3, _mm_div_ss(w, w));
}

void Unaligned_Vec4HomogenizeToVec3(const float* __restrict a, float* __restrict b) {
  __m128 x = _mm_load_ss(a);
  __m128 y = _mm_load_ss(a + 1);
  __m128 z = _mm_load_ss(a + 2);
  __m128 w = _mm_load_ss(a + 3);
  _mm_store_ss(b, _mm_div_ss(x, w));
  _mm_store_ss(b + 1, _mm_div_ss(y, w));
  _mm_store_ss(b + 2, _mm_div_ss(z, w));
}

void Unaligned_Mat4x4Vec4Transform(const float matrix[4][4], const float* __restrict a, float* __restrict b) {
  __m128 matrixX = _mm_loadu_ps(matrix[0]);
  __m128 matrixY = _mm_loadu_ps(matrix[1]);
  __m128 matrixZ = _mm_loadu_ps(matrix[2]);
  __m128 matrixW = _mm_loadu_ps(matrix[3]);

  __m128 loXY = _mm_unpacklo_ps(matrixX, matrixY);
  __m128 loZW = _mm_unpacklo_ps(matrixZ, matrixW);
  __m128 hiXY = _mm_unpackhi_ps(matrixX, matrixY);
  __m128 hiZW = _mm_unpackhi_ps(matrixZ, matrixW);

  matrixX = _mm_shuffle_ps(loXY, loZW, 0b01000100);
  matrixY = _mm_shuffle_ps(loXY, loZW, 0b11101110);
  matrixZ = _mm_shuffle_ps(hiXY, hiZW, 0b01000100);
  matrixW = _mm_shuffle_ps(hiXY, hiZW, 0b11101110);

  __m128 xyzw = _mm_loadu_ps(a);

  __m128 vecX = _mm_shuffle_ps(xyzw, xyzw, 0b00000000);
  __m128 vecY = _mm_shuffle_ps(xyzw, xyzw, 0b01010101);
  __m128 vecZ = _mm_shuffle_ps(xyzw, xyzw, 0b10101010);
  __m128 vecW = _mm_shuffle_ps(xyzw, xyzw, 0b11111111);

  __m128 result = _mm_add_ps(_mm_mul_ps(matrixX, vecX), _mm_mul_ps(matrixY, vecY));
  result = _mm_add_ps(result, _mm_mul_ps(matrixZ, vecZ));
  result = _mm_add_ps(result, _mm_mul_ps(matrixW, vecW));

  _mm_storeu_ps(b, result);
}

void Unaligned_Mat4x4Vec4TransformInPlace(const float matrix[4][4], float* a) {
  __m128 matrixX = _mm_loadu_ps(matrix[0]);
  __m128 matrixY = _mm_loadu_ps(matrix[1]);
  __m128 matrixZ = _mm_loadu_ps(matrix[2]);
  __m128 matrixW = _mm_loadu_ps(matrix[3]);

  __m128 loXY = _mm_unpacklo_ps(matrixX, matrixY);
  __m128 loZW = _mm_unpacklo_ps(matrixZ, matrixW);
  __m128 hiXY = _mm_unpackhi_ps(matrixX, matrixY);
  __m128 hiZW = _mm_unpackhi_ps(matrixZ, matrixW);

  matrixX = _mm_shuffle_ps(loXY, loZW, 0b01000100);
  matrixY = _mm_shuffle_ps(loXY, loZW, 0b11101110);
  matrixZ = _mm_shuffle_ps(hiXY, hiZW, 0b01000100);
  matrixW = _mm_shuffle_ps(hiXY, hiZW, 0b11101110);

  __m128 xyzw = _mm_loadu_ps(a);

  __m128 vecX = _mm_shuffle_ps(xyzw, xyzw, 0b00000000);
  __m128 vecY = _mm_shuffle_ps(xyzw, xyzw, 0b01010101);
  __m128 vecZ = _mm_shuffle_ps(xyzw, xyzw, 0b10101010);
  __m128 vecW = _mm_shuffle_ps(xyzw, xyzw, 0b11111111);

  __m128 result = _mm_add_ps(_mm_mul_ps(matrixX, vecX), _mm_mul_ps(matrixY, vecY));
  result = _mm_add_ps(result, _mm_mul_ps(matrixZ, vecZ));
  result = _mm_add_ps(result, _mm_mul_ps(matrixW, vecW));

  _mm_storeu_ps(a, result);
}

float Unaligned_ParametricLinePlaneIntersection(const float start[4], const float end[4], const float edgeNormal[4], const float edgePoint[4]) {
  __m128 startVec = _mm_loadu_ps(start);
  __m128 endVec = _mm_loadu_ps(end);
  __m128 normalVec = _mm_loadu_ps(edgeNormal);
  __m128 pointVec = _mm_loadu_ps(edgePoint);
  __m128 signFlip = _mm_set_ps1(-0.f); 

  __m128 numeratorIntermediate = _mm_mul_ps(_mm_sub_ps(startVec, pointVec), normalVec);
  numeratorIntermediate = _mm_hadd_ps(_mm_hadd_ps(numeratorIntermediate, numeratorIntermediate), _mm_setzero_ps());

  __m128 denominatorIntermediate = _mm_mul_ps(_mm_sub_ps(endVec, startVec), _mm_xor_ps(normalVec, signFlip));
  denominatorIntermediate = _mm_hadd_ps(_mm_hadd_ps(denominatorIntermediate, denominatorIntermediate), _mm_setzero_ps());

  return _mm_cvtss_f32(_mm_div_ss(numeratorIntermediate, denominatorIntermediate));
}

bool Unaligned_InsidePlane(const float point[4], const float clipEdge[4], const float normal[4]) {
  __m128 pointVec = _mm_loadu_ps(point);
  __m128 clipVec = _mm_loadu_ps(clipEdge);
  __m128 normalVec = _mm_loadu_ps(normal);
  __m128i dotSign = _mm_set_epi32(0, 0, 0, 0x80000000);
  __m128 dotIntermediate = _mm_mul_ps(_mm_sub_ps(pointVec, clipVec), normalVec);
  dotIntermediate = _mm_hadd_ps(_mm_hadd_ps(dotIntermediate, dotIntermediate), _mm_setzero_ps());
  return !_mm_testz_si128(_mm_castps_si128(dotIntermediate), dotSign);
}

void Unaligned_ParametricVector4D(const float point, const float start[4], const float end[4], float outVec[4]) {
  __m128 endVec = _mm_loadu_ps(end);
  __m128 startVec = _mm_loadu_ps(start);
  __m128 pointVec = _mm_set_ps1(point);
  _mm_storeu_ps(outVec, _mm_add_ps(_mm_mul_ps(_mm_sub_ps(endVec, startVec), pointVec), startVec));
}

void Unaligned_LerpAttribute(const float attributeA[4], const float attributeB[4], float outAttribute[4], float parametricT) {
  _mm_storeu_ps(outAttribute, Lerp128(_mm_loadu_ps(attributeA), _mm_loadu_ps(attributeB), _mm_set_ps1(parametricT)));
}

void Unaligned_Mat4x4Mul(const float* a, const float* b, float* result) {
  __m128 a0 = _mm_set_ps1(a[0]);
  __m128 a1 = _mm_set_ps1(a[1]);
  __m128 a2 = _mm_set_ps1(a[2]);
  __m128 a3 = _mm_set_ps1(a[3]);

  __m128 a4 = _mm_set_ps1(a[4]);
  __m128 a5 = _mm_set_ps1(a[5]);
  __m128 a6 = _mm_set_ps1(a[6]);
  __m128 a7 = _mm_set_ps1(a[7]);

  __m128 a8 = _mm_set_ps1(a[8]);
  __m128 a9 = _mm_set_ps1(a[9]);
  __m128 a10 = _mm_set_ps1(a[10]);
  __m128 a11 = _mm_set_ps1(a[11]);

  __m128 a12 = _mm_set_ps1(a[12]);
  __m128 a13 = _mm_set_ps1(a[13]);
  __m128 a14 = _mm_set_ps1(a[14]);
  __m128 a15 = _mm_set_ps1(a[15]);

  __m128 br0 = _mm_loadu_ps(b);
  __m128 br1 = _mm_loadu_ps(b + 4);
  __m128 br2 = _mm_loadu_ps(b + 8);
  __m128 br3 = _mm_loadu_ps(b + 12);

  __m128 result0 = _mm_mul_ps(a0, br0);
  __m128 result1 = _mm_mul_ps(a1, br1);
  __m128 result2 = _mm_mul_ps(a2, br2);
  __m128 result3 = _mm_mul_ps(a3, br3);

  __m128 result4 = _mm_mul_ps(a4, br0);
  __m128 result5 = _mm_mul_ps(a5, br1);
  __m128 result6 = _mm_mul_ps(a6, br2);
  __m128 result7 = _mm_mul_ps(a7, br3);

  __m128 result8 = _mm_mul_ps(a8, br0);
  __m128 result9 = _mm_mul_ps(a9, br1);
  __m128 result10 = _mm_mul_ps(a10, br2);
  __m128 result11 = _mm_mul_ps(a11, br3);

  __m128 result12 = _mm_mul_ps(a12, br0);
  __m128 result13 = _mm_mul_ps(a13, br1);
  __m128 result14 = _mm_mul_ps(a14, br2);
  __m128 result15 = _mm_mul_ps(a15, br3);

  _mm_store_ps(result, _mm_add_ps(_mm_add_ps(_mm_add_ps(result0, result1), result2), result3));
  _mm_store_ps(result + 4, _mm_add_ps(_mm_add_ps(_mm_add_ps(result4, result5), result6), result7));
  _mm_store_ps(result + 8, _mm_add_ps(_mm_add_ps(_mm_add_ps(result8, result9), result10), result11));
  _mm_store_ps(result + 12, _mm_add_ps(_mm_add_ps(_mm_add_ps(result12, result13), result14), result15));
}

void Unaligned_Mat4x4MulInPlace(float* a, const float* b) {
  __m128 a0 = _mm_set_ps1(a[0]);
  __m128 a1 = _mm_set_ps1(a[1]);
  __m128 a2 = _mm_set_ps1(a[2]);
  __m128 a3 = _mm_set_ps1(a[3]);

  __m128 a4 = _mm_set_ps1(a[4]);
  __m128 a5 = _mm_set_ps1(a[5]);
  __m128 a6 = _mm_set_ps1(a[6]);
  __m128 a7 = _mm_set_ps1(a[7]);

  __m128 a8 = _mm_set_ps1(a[8]);
  __m128 a9 = _mm_set_ps1(a[9]);
  __m128 a10 = _mm_set_ps1(a[10]);
  __m128 a11 = _mm_set_ps1(a[11]);

  __m128 a12 = _mm_set_ps1(a[12]);
  __m128 a13 = _mm_set_ps1(a[13]);
  __m128 a14 = _mm_set_ps1(a[14]);
  __m128 a15 = _mm_set_ps1(a[15]);

  __m128 br0 = _mm_loadu_ps(b);
  __m128 br1 = _mm_loadu_ps(b + 4);
  __m128 br2 = _mm_loadu_ps(b + 8);
  __m128 br3 = _mm_loadu_ps(b + 12);

  __m128 result0 = _mm_mul_ps(a0, br0);
  __m128 result1 = _mm_mul_ps(a1, br1);
  __m128 result2 = _mm_mul_ps(a2, br2);
  __m128 result3 = _mm_mul_ps(a3, br3);

  __m128 result4 = _mm_mul_ps(a4, br0);
  __m128 result5 = _mm_mul_ps(a5, br1);
  __m128 result6 = _mm_mul_ps(a6, br2);
  __m128 result7 = _mm_mul_ps(a7, br3);

  __m128 result8 = _mm_mul_ps(a8, br0);
  __m128 result9 = _mm_mul_ps(a9, br1);
  __m128 result10 = _mm_mul_ps(a10, br2);
  __m128 result11 = _mm_mul_ps(a11, br3);

  __m128 result12 = _mm_mul_ps(a12, br0);
  __m128 result13 = _mm_mul_ps(a13, br1);
  __m128 result14 = _mm_mul_ps(a14, br2);
  __m128 result15 = _mm_mul_ps(a15, br3);

  _mm_store_ps(a, _mm_add_ps(_mm_add_ps(_mm_add_ps(result0, result1), result2), result3));
  _mm_store_ps(a + 4, _mm_add_ps(_mm_add_ps(_mm_add_ps(result4, result5), result6), result7));
  _mm_store_ps(a + 8, _mm_add_ps(_mm_add_ps(_mm_add_ps(result8, result9), result10), result11));
  _mm_store_ps(a + 12, _mm_add_ps(_mm_add_ps(_mm_add_ps(result12, result13), result14), result15));
}

void Unaligned_RGBXToBGRA(const uint8* rgb, uint8* rgba, size_t rgbBytes) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    // [RGBX, RGBX, RGBX, RGBX, RGBX, RGBX, RGBX, RGBX] => [BGRA, BGRA, BGRA, BGRA, BGRA, BGRA, BGRA, BGRA]
    __m256i shuffleOrder = _mm256_set_epi8(
      31, 28, 29, 30,
      27, 24, 25, 26,
      23, 20, 21, 22,
      19, 16, 17, 18,
      15, 12, 13, 14,
      11, 8, 9, 10,
      7, 4, 5, 6,
      3, 0, 1, 2);

    size_t i = 0;
    size_t j = 0;
    __m256i alpha = _mm256_set1_epi32(0xFF000000);

    const size_t simdLength = (rgbBytes >> 3) << 3;
    for (; j < simdLength; i += 8, j += 8) {
      __m256i inData = _mm256_lddqu_si256((__m256i*)(rgb + i));
      __m256i shuffledPixels = _mm256_shuffle_epi8(inData, shuffleOrder);
      shuffledPixels = _mm256_or_si256(shuffledPixels, alpha);
      _mm256_storeu_si256((__m256i*)(rgba + j), shuffledPixels);
    }

    for (; i < rgbBytes; i += 4, j += 4) {
      rgba[j] = rgb[i + 2];
      rgba[j + 1] = rgb[i + 1];
      rgba[j + 2] = rgb[i];
      rgba[j + 3] = 0xFF;
    }
  }
  else {
    // [RGBX, RGBX, RGBX, RGBX] => [BGRA, BGRA, BGRA, BGRA]
    __m128i shuffleOrder = _mm_set_epi8(
      15, 12, 13, 14,
      11, 8, 9, 10,
      7, 4, 5, 6,
      3, 0, 1, 2);

    size_t i = 0;
    size_t j = 0;
    __m128i alpha = _mm_set1_epi32(0xFF000000);

    const size_t simdLength = (rgbBytes >> 2) << 2;
    for (; i < simdLength; i += 4, j += 4) {
      __m128i inData = _mm_lddqu_si128((__m128i*)(rgb + i));
      __m128i shuffledPixels = _mm_shuffle_epi8(inData, shuffleOrder);
      shuffledPixels = _mm_or_si128(shuffledPixels, alpha);
      _mm_storeu_si128((__m128i*)(rgba + j), shuffledPixels);
    }

    for (; i < rgbBytes; i += 4, j += 4) {
      rgba[j] = rgb[i + 2];
      rgba[j + 1] = rgb[i + 1];
      rgba[j + 2] = rgb[i];
      rgba[j + 3] = 0xFF;
    }
  }
}

void Unaligned_BGRToBGRA(const uint8* rgb, uint8* rgba, size_t rgbBytes) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    // [BGRB, GRBG, RBGR, BGRB, GRBG, RBGR, BGRB, GRBG] => [BGRA, BGRA, BGRA, BGRA, BGRA, BGRA, BGRA, BGRA]
    __m256i shuffleOrder = _mm256_set_epi8(
      23, 23, 22, 21,
      20, 20, 19, 18,
      17, 17, 16, 15,
      14, 14, 13, 12,
      11, 11, 10, 9,
      8, 8, 7, 6,
      5, 5, 4, 3,
      2, 2, 1, 0);

    size_t i = 0;
    size_t j = 0;
    __m256i alpha = _mm256_set1_epi32(0xFF000000);

    const size_t simdLength = (rgbBytes >> 3) << 3;
    for (; i < simdLength; i += 6, j += 8) {
      __m256i inData = _mm256_lddqu_si256((__m256i*)(rgb + i));
      __m256i shuffledPixels = _mm256_shuffle_epi8(inData, shuffleOrder);
      shuffledPixels = _mm256_or_si256(shuffledPixels, alpha);
      _mm256_storeu_si256((__m256i*)(rgba + j), shuffledPixels);
    }

    for (; i < rgbBytes; i += 3, j += 4) {
      rgba[j] = rgb[i];
      rgba[j + 1] = rgb[i + 1];
      rgba[j + 2] = rgb[i + 2];
      rgba[j + 3] = 0xFF;
    }
  }
  else {
    // [BGRB, GRBG, RBGR, BGRB] => [BGRA, BGRA, BGRA, BGRA]
    __m128i shuffleOrder = _mm_set_epi8(
      11, 11, 10, 9,
      8, 8, 7, 6,
      5, 5, 4, 3,
      2, 2, 1, 0);

    size_t i = 0;
    size_t j = 0;
    __m128i alpha = _mm_set1_epi32(0xFF000000);

    const size_t simdLength = (rgbBytes >> 2) << 2;
    for (; i < simdLength; i += 3, j += 4) {
      __m128i inData = _mm_lddqu_si128((__m128i*)(rgb + i));
      __m128i shuffledPixels = _mm_shuffle_epi8(inData, shuffleOrder);
      shuffledPixels = _mm_or_si128(shuffledPixels, alpha);
      _mm_storeu_si128((__m128i*)(rgba + j), shuffledPixels);
    }

    for (; i < rgbBytes; i += 3, j += 4) {
      rgba[j] = rgb[i];
      rgba[j + 1] = rgb[i + 1];
      rgba[j + 2] = rgb[i + 2];
      rgba[j + 3] = 0xFF;
    }
  }
}

void Unaligned_RGBAToBGRA(uint32* image, size_t width, size_t height) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    __m256i shuffleOrder = _mm256_set_epi8(
      15, 12, 13, 14,
      11, 8, 9, 10,
      7, 4, 5, 6,
      3, 0, 1, 2,
      15, 12, 13, 14,
      11, 8, 9, 10,
      7, 4, 5, 6,
      3, 0, 1, 2);

    const size_t simdLength = (width >> 3) << 3;
    for (size_t h = 0; h < height; ++h) {
      uint32* currentPixels = image + (h * width);

      size_t w = 0;
      for (; w < simdLength; w += 8, currentPixels += 8) {
        __m256i pixels = _mm256_loadu_si256((__m256i*)(currentPixels));
        pixels = _mm256_shuffle_epi8(pixels, shuffleOrder);
        _mm256_storeu_si256((__m256i*)(currentPixels), pixels);
      }

      for (; w < width; ++w, ++currentPixels) {
        uint8* channels = (uint8*)currentPixels;
        uint8 blueValue = channels[2];
        uint8 redValue = channels[0];
        channels[0] = blueValue;
        channels[2] = redValue;
      }
    }
  }
  else {
    __m128i shuffleOrder = _mm_set_epi8(
      15, 12, 13, 14,
      11, 8, 9, 10,
      7, 4, 5, 6,
      3, 0, 1, 2);

    const size_t simdLength = (width >> 2) << 2;
    for (size_t h = 0; h < height; ++h) {
      uint32* currentPixels = image + (h * width);

      size_t w = 0;
      for (; w < simdLength; w += 4, currentPixels += 4) {
        __m128i pixels = _mm_loadu_si128((__m128i*)(currentPixels));
        pixels = _mm_shuffle_epi8(pixels, shuffleOrder);
        _mm_storeu_si128((__m128i*)(currentPixels), pixels);
      }

      for (; w < width; ++w, ++currentPixels) {
        uint8* channels = (uint8*)currentPixels;
        uint8 blueValue = channels[2];
        uint8 redValue = channels[0];
        channels[0] = blueValue;
        channels[2] = redValue;
      }
    }
  }
}

void Unaligned_BilinearScaleImage_SSE(uint8* __restrict source, size_t sourceWidth, size_t sourceHeight, uint8* __restrict dest, size_t destWidth, size_t destHeight) {
  float ratioX = ((float)sourceWidth - 1) / ((float)destWidth - 1);
  float ratioY = ((float)sourceHeight - 1) / ((float)destHeight - 1);

  size_t sourceStride = sourceWidth * 4;
  size_t destStride = destWidth * 4;

  for (size_t y = 0; y < destHeight; ++y) {
    uint8* destRow = dest + (y * destStride);
    for (size_t x = 0; x < destWidth; ++x) {
      float xRatio = ratioX * x;
      float yRatio = ratioY * y;

      size_t xLeft = (size_t)floorf(xRatio);
      size_t yLeft = (size_t)floorf(yRatio);
      size_t xRight = (size_t)ceilf(xRatio);
      size_t yRight = (size_t)ceilf(yRatio);

      float xWeight = xRatio - xLeft;
      float yWeight = yRatio - yLeft;

      uint8* topLeft = source + (yLeft * sourceStride) + (xLeft * 4);
      uint8* topRight = source + (yLeft * sourceStride) + (xRight * 4);
      uint8* bottomLeft = source + (yRight * sourceStride) + (xLeft * 4);
      uint8* bottomRight = source + (yRight * sourceStride) + (xRight * 4);

      float topLeftScale = (1.f - xWeight) * (1.f - yWeight);
      float topRightScale = xWeight * (1.f - yWeight);
      float bottomLeftScale = yWeight * (1.f - xWeight);
      float bottomRightScale = xWeight * yWeight;

      destRow[(x * 4)] = (uint8)(topLeft[0] * topLeftScale +
        topRight[0] * topRightScale +
        bottomLeft[0] * bottomLeftScale +
        bottomRight[0] * bottomRightScale);
      destRow[(x * 4) + 1] = (uint8)(topLeft[1] * topLeftScale +
        topRight[1] * topRightScale +
        bottomLeft[1] * bottomLeftScale +
        bottomRight[1] * bottomRightScale);
      destRow[(x * 4) + 2] = (uint8)(topLeft[2] * topLeftScale +
        topRight[2] * topRightScale +
        bottomLeft[2] * bottomLeftScale +
        bottomRight[2] * bottomRightScale);
      destRow[(x * 4) + 3] = (uint8)(topLeft[3] * topLeftScale +
        topRight[3] * topRightScale +
        bottomLeft[3] * bottomLeftScale +
        bottomRight[3] * bottomRightScale);
    }
  }
}

void Unaligned_BilinearScaleImage_AVX(uint8* __restrict source, size_t sourceWidth, size_t sourceHeight, uint8* __restrict dest, size_t destWidth, size_t destHeight) {
  float ratioXValue = ((float)sourceWidth - 1) / ((float)destWidth - 1);
  float ratioYValue = ((float)sourceHeight - 1) / ((float)destHeight - 1);
  __m256 ratioX = _mm256_set1_ps(ratioXValue);
  __m256 ratioY = _mm256_set1_ps(ratioYValue);

  __m256 xScale = _mm256_set1_ps(8.f);
  const size_t sourceStride = sourceWidth * 4;
  __m256 strideScale = _mm256_set1_ps((float)(sourceStride));
  __m256 indexScale = _mm256_set1_ps(4.f);
  __m256 lerpOne = _mm256_set1_ps(1.f);

  __m256i bShuffle = _mm256_set_epi8(
    0x80U, 0x80U, 0x80U, 12,
    0x80U, 0x80U, 0x80U, 8,
    0x80U, 0x80U, 0x80U, 4,
    0x80U, 0x80U, 0x80U, 0,
    0x80U, 0x80U, 0x80U, 12,
    0x80U, 0x80U, 0x80U, 8,
    0x80U, 0x80U, 0x80U, 4,
    0x80U, 0x80U, 0x80U, 0
  );

  __m256i gShuffle = _mm256_set_epi8(
    0x80U, 0x80U, 0x80U, 13,
    0x80U, 0x80U, 0x80U, 9,
    0x80U, 0x80U, 0x80U, 5,
    0x80U, 0x80U, 0x80U, 1,
    0x80U, 0x80U, 0x80U, 13,
    0x80U, 0x80U, 0x80U, 9,
    0x80U, 0x80U, 0x80U, 5,
    0x80U, 0x80U, 0x80U, 1
  );

  __m256i rShuffle = _mm256_set_epi8(
    0x80U, 0x80U, 0x80U, 14,
    0x80U, 0x80U, 0x80U, 10,
    0x80U, 0x80U, 0x80U, 6,
    0x80U, 0x80U, 0x80U, 2,
    0x80U, 0x80U, 0x80U, 14,
    0x80U, 0x80U, 0x80U, 10,
    0x80U, 0x80U, 0x80U, 6,
    0x80U, 0x80U, 0x80U, 2
  );

  __m256i aShuffle = _mm256_set_epi8(
    0x80U, 0x80U, 0x80U, 15,
    0x80U, 0x80U, 0x80U, 11,
    0x80U, 0x80U, 0x80U, 7,
    0x80U, 0x80U, 0x80U, 3,
    0x80U, 0x80U, 0x80U, 15,
    0x80U, 0x80U, 0x80U, 11,
    0x80U, 0x80U, 0x80U, 7,
    0x80U, 0x80U, 0x80U, 3
  );

  const size_t simdLength = (destWidth >> 3) << 3;
  for (size_t y = 0; y < destHeight; ++y) {
    uint8* destRow = dest + (y * destWidth * 4);
    __m256 yValues = _mm256_set1_ps((float)y);

    size_t x = 0;
    for (__m256 xValues = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f); x < simdLength; x += 8, xValues = _mm256_add_ps(xValues, xScale)) {
      __m256 xRatios = _mm256_mul_ps(ratioX, xValues);
      __m256 yRatios = _mm256_mul_ps(ratioY, yValues);

      __m256 xLeft = _mm256_floor_ps(xRatios);
      __m256 yLeft = _mm256_floor_ps(yRatios);
      __m256 xRight = _mm256_ceil_ps(xRatios);
      __m256 yRight = _mm256_ceil_ps(yRatios);

      __m256 xWeight = _mm256_sub_ps(xRatios, xLeft);
      __m256 yWeight = _mm256_sub_ps(yRatios, yLeft);

      __m256 xLeftInt = _mm256_mul_ps(xLeft, indexScale);
      __m256 yLeftInt = yLeft;
      __m256 xRightInt = _mm256_mul_ps(xRight, indexScale);
      __m256 yRightInt = yRight;

      __m256i topLeftIndices = _mm256_cvtps_epi32(_mm256_fmadd_ps(yLeftInt, strideScale, xLeftInt));
      __m256i topRightIndices = _mm256_cvtps_epi32(_mm256_fmadd_ps(yLeftInt, strideScale, xRightInt));
      __m256i bottomLeftIndices = _mm256_cvtps_epi32(_mm256_fmadd_ps(yRightInt, strideScale, xLeftInt));
      __m256i bottomRightIndices = _mm256_cvtps_epi32(_mm256_fmadd_ps(yRightInt, strideScale, xRightInt));

      __m256i topLeft = _mm256_i32gather_epi32((int32*)source, topLeftIndices, 1);
      __m256i topRight = _mm256_i32gather_epi32((int32*)source, topRightIndices, 1);
      __m256i bottomLeft = _mm256_i32gather_epi32((int32*)source, bottomLeftIndices, 1);
      __m256i bottomRight = _mm256_i32gather_epi32((int32*)source, bottomRightIndices, 1);

      __m256 topLeftScale = _mm256_mul_ps(_mm256_sub_ps(lerpOne, xWeight), _mm256_sub_ps(lerpOne, yWeight));
      __m256 topRightScale = _mm256_mul_ps(xWeight, _mm256_sub_ps(lerpOne, yWeight));
      __m256 bottomLeftScale = _mm256_mul_ps(yWeight, _mm256_sub_ps(lerpOne, xWeight));
      __m256 bottomRightScale = _mm256_mul_ps(xWeight, yWeight);

      __m256i topLeftColor = _mm256_cvtps_epi32(
        _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topLeft, aShuffle)), topLeftScale,
          _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topRight, aShuffle)), topRightScale,
            _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomLeft, aShuffle)), bottomLeftScale,
              _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomRight, aShuffle)), bottomRightScale))))
      );

      __m256i topRightColor = _mm256_cvtps_epi32(
        _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topLeft, rShuffle)), topLeftScale,
          _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topRight, rShuffle)), topRightScale,
            _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomLeft, rShuffle)), bottomLeftScale,
              _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomRight, rShuffle)), bottomRightScale))))
      );

      __m256i bottomLeftColor = _mm256_cvtps_epi32(
        _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topLeft, gShuffle)), topLeftScale,
          _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topRight, gShuffle)), topRightScale,
            _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomLeft, gShuffle)), bottomLeftScale,
              _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomRight, gShuffle)), bottomRightScale))))
      );

      __m256i bottomRightColor = _mm256_cvtps_epi32(
        _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topLeft, bShuffle)), topLeftScale,
          _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(topRight, bShuffle)), topRightScale,
            _mm256_fmadd_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomLeft, bShuffle)), bottomLeftScale,
              _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_shuffle_epi8(bottomRight, bShuffle)), bottomRightScale))))
      );

      topLeftColor = _mm256_slli_epi32(topLeftColor, 24);
      topRightColor = _mm256_slli_epi32(topRightColor, 16);
      bottomLeftColor = _mm256_slli_epi32(bottomLeftColor, 8);

      __m256i finalColor = _mm256_or_si256(_mm256_or_si256(_mm256_or_si256(topLeftColor, topRightColor), bottomLeftColor), bottomRightColor);

      _mm256_storeu_si256((__m256i*)(destRow + (x * 4)), finalColor);
    }

    for (; x < destWidth; ++x) {
      float xRatio = ratioXValue * x;
      float yRatio = ratioYValue * y;

      size_t xLeft = (size_t)floorf(xRatio);
      size_t yLeft = (size_t)floorf(yRatio);
      size_t xRight = (size_t)ceilf(xRatio);
      size_t yRight = (size_t)ceilf(yRatio);

      float xWeight = xRatio - xLeft;
      float yWeight = yRatio - yLeft;

      uint8* topLeft = source + (yLeft * sourceStride) + (xLeft * 4);
      uint8* topRight = source + (yLeft * sourceStride) + (xRight * 4);
      uint8* bottomLeft = source + (yRight * sourceStride) + (xLeft * 4);
      uint8* bottomRight = source + (yRight * sourceStride) + (xRight * 4);

      float topLeftScale = (1.f - xWeight) * (1.f - yWeight);
      float topRightScale = xWeight * (1.f - yWeight);
      float bottomLeftScale = yWeight * (1.f - xWeight);
      float bottomRightScale = xWeight * yWeight;

      destRow[(x * 4)] = (uint8)(topLeft[0] * topLeftScale +
        topRight[0] * topRightScale +
        bottomLeft[0] * bottomLeftScale +
        bottomRight[0] * bottomRightScale);
      destRow[(x * 4) + 1] = (uint8)(topLeft[1] * topLeftScale +
        topRight[1] * topRightScale +
        bottomLeft[1] * bottomLeftScale +
        bottomRight[1] * bottomRightScale);
      destRow[(x * 4) + 2] = (uint8)(topLeft[2] * topLeftScale +
        topRight[2] * topRightScale +
        bottomLeft[2] * bottomLeftScale +
        bottomRight[2] * bottomRightScale);
      destRow[(x * 4) + 3] = (uint8)(topLeft[3] * topLeftScale +
        topRight[3] * topRightScale +
        bottomLeft[3] * bottomLeftScale +
        bottomRight[3] * bottomRightScale);
    }
  }
}

void Unaligned_GenerateMipLevel_SSE(uint8* __restrict nextMip, size_t nextWidth, size_t nextHeight, uint8* __restrict lastMip, size_t lastWidth, size_t lastHeight) {
  (void)lastHeight;
  size_t nextMipStride = nextWidth * 4;
  size_t lastMipStride = lastWidth * 4;

  __m128i shuffleLeftWide = _mm_set_epi8(
    0x80U, 11, 0x80U, 10,
    0x80U, 9, 0x80U, 8,
    0x80U, 3, 0x80U, 2,
    0x80U, 1, 0x80U, 0
  );

  __m128i shuffleRightWide = _mm_set_epi8(
    0x80U, 15, 0x80U, 14,
    0x80U, 13, 0x80U, 12,
    0x80U, 7, 0x80U, 6,
    0x80U, 5, 0x80U, 4
  );

  const size_t simdSize = (nextWidth >> 1) << 1;
  for (size_t y = 0; y < nextHeight; ++y) {
    size_t x = 0;
    for (; x < simdSize; x += 2) {
      const size_t xStride = x * 4;

      uint8* __restrict topLeft = lastMip + (y * 2 * lastMipStride) + (xStride * 2);
      uint8* __restrict bottomLeft = topLeft + lastMipStride;

      __m128i topData = _mm_lddqu_si128((__m128i*)topLeft);
      __m128i bottomData = _mm_lddqu_si128((__m128i*)bottomLeft);

      __m128i topLeftData = _mm_shuffle_epi8(topData, shuffleLeftWide);
      __m128i topRightData = _mm_shuffle_epi8(topData, shuffleRightWide);
      __m128i bottomLeftData = _mm_shuffle_epi8(bottomData, shuffleLeftWide);
      __m128i bottomRightData = _mm_shuffle_epi8(bottomData, shuffleRightWide);

      __m128i rgba = _mm_avg_epu16(_mm_avg_epu16(topLeftData, topRightData), _mm_avg_epu16(bottomLeftData, bottomRightData));
      rgba = _mm_packus_epi16(rgba, rgba);

      _mm_storeu_si64(nextMip + ((y * nextMipStride) + xStride), rgba);
    }

    for (; x < nextWidth; ++x) {
      const size_t xStride = x * 4;

      uint8* __restrict topLeft = lastMip + (y * 2 * lastMipStride) + (xStride * 2);
      uint8* __restrict bottomLeft = topLeft + lastMipStride;

      __m128i topData = _mm_loadu_si64(topLeft);
      __m128i bottomData = _mm_loadu_si64(bottomLeft);

      __m128i topLeftData = _mm_shuffle_epi8(topData, shuffleLeftWide);
      __m128i topRightData = _mm_shuffle_epi8(topData, shuffleRightWide);
      __m128i bottomLeftData = _mm_shuffle_epi8(bottomData, shuffleLeftWide);
      __m128i bottomRightData = _mm_shuffle_epi8(bottomData, shuffleRightWide);

      __m128i rgba = _mm_avg_epu16(_mm_avg_epu16(topLeftData, topRightData), _mm_avg_epu16(bottomLeftData, bottomRightData));
      rgba = _mm_packus_epi16(rgba, rgba);

      _mm_storeu_si32(nextMip + ((y * nextMipStride) + xStride), rgba);
    }
  }
}

void Unaligned_GenerateMipLevel_AVX(uint8* __restrict nextMip, size_t nextWidth, size_t nextHeight, uint8* __restrict lastMip, size_t lastWidth, size_t lastHeight) {
  (void)lastHeight;
  size_t nextMipStride = nextWidth * 4;
  size_t lastMipStride = lastWidth * 4;

  __m256i shuffleLeftWide = _mm256_set_epi8(
    0x80U, 11, 0x80U, 10,
    0x80U, 9, 0x80U, 8,
    0x80U, 3, 0x80U, 2,
    0x80U, 1, 0x80U, 0,
    0x80U, 11, 0x80U, 10,
    0x80U, 9, 0x80U, 8,
    0x80U, 3, 0x80U, 2,
    0x80U, 1, 0x80U, 0
  );

  __m256i shuffleRightWide = _mm256_set_epi8(
    0x80U, 15, 0x80U, 14,
    0x80U, 13, 0x80U, 12,
    0x80U, 7, 0x80U, 6,
    0x80U, 5, 0x80U, 4,
    0x80U, 15, 0x80U, 14,
    0x80U, 13, 0x80U, 12,
    0x80U, 7, 0x80U, 6,
    0x80U, 5, 0x80U, 4
  );

  const size_t simdSize = (nextWidth >> 2) << 2;
  for (size_t y = 0; y < nextHeight; ++y) {
    size_t x = 0;
    for (; x < simdSize; x += 4) {
      const size_t xStride = x * 4;

      uint8* __restrict topLeft = lastMip + (y * 2 * lastMipStride) + (xStride * 2);
      uint8* __restrict bottomLeft = topLeft + lastMipStride;

      __m256i topData = _mm256_lddqu_si256((__m256i*)topLeft);
      __m256i bottomData = _mm256_lddqu_si256((__m256i*)bottomLeft);

      __m256i topLeftData = _mm256_shuffle_epi8(topData, shuffleLeftWide);
      __m256i topRightData = _mm256_shuffle_epi8(topData, shuffleRightWide);
      __m256i bottomLeftData = _mm256_shuffle_epi8(bottomData, shuffleLeftWide);
      __m256i bottomRightData = _mm256_shuffle_epi8(bottomData, shuffleRightWide);

      __m256i rgba = _mm256_avg_epu16(_mm256_avg_epu16(topLeftData, topRightData), _mm256_avg_epu16(bottomLeftData, bottomRightData));
      __m128i hiRgba = _mm_packus_epi16(_mm256_castsi256_si128(rgba), _mm256_extractf128_si256(rgba, 0b1));

      _mm_storeu_si128((__m128i*)(nextMip + ((y * nextMipStride) + xStride)), hiRgba);
    }

    for (; x < nextWidth; ++x) {
      const size_t xStride = x * 4;

      uint8* __restrict topLeft = lastMip + (y * 2 * lastMipStride) + (xStride * 2);
      uint8* __restrict bottomLeft = topLeft + lastMipStride;

      __m128i topData = _mm_loadu_si64(topLeft);
      __m128i bottomData = _mm_loadu_si64(bottomLeft);

      __m128i topLeftData = _mm_shuffle_epi8(topData, _mm256_castsi256_si128(shuffleLeftWide));
      __m128i topRightData = _mm_shuffle_epi8(topData, _mm256_castsi256_si128(shuffleRightWide));
      __m128i bottomLeftData = _mm_shuffle_epi8(bottomData, _mm256_castsi256_si128(shuffleLeftWide));
      __m128i bottomRightData = _mm_shuffle_epi8(bottomData, _mm256_castsi256_si128(shuffleRightWide));

      __m128i rgba = _mm_avg_epu16(_mm_avg_epu16(topLeftData, topRightData), _mm_avg_epu16(bottomLeftData, bottomRightData));
      rgba = _mm_packus_epi16(rgba, rgba);

      _mm_storeu_si32(nextMip + ((y * nextMipStride) + xStride), rgba);
    }
  }
}

void Unaligned_DrawDebugText_SSE(const uint8 lut[128][8], const String& message, size_t x, size_t y, uint8* buffer, size_t width, const ZColor& color) {
  size_t xOffset = x;
  size_t yOffset = y;

  const char* curChar = message.Str();
  const size_t stride = width * sizeof(uint32);
  const uint32 colorValue = color.Color();

  __m128i bitMaskLo = _mm_set_epi32(1 << 3, 1 << 2, 1 << 1, 1);
  __m128i bitMaskHi = _mm_set_epi32(1 << 7, 1 << 6, 1 << 5, 1 << 4);
  __m128i shiftMaskLo = _mm_set_epi32(28, 29, 30, 31);
  __m128i shiftMaskHi = _mm_set_epi32(24, 25, 26, 27);
  __m128i colors = _mm_set1_epi32(colorValue);

  for (size_t strLen = 0; strLen < message.Length(); ++strLen) {
    const char fontChar = *curChar;
    __m128i fontRow = _mm_loadu_si64(lut[(uint8)fontChar]);
    for (size_t curX = 0; curX < 8; ++curX) {
      uint32* bufferPosition = (uint32*)(buffer + (xOffset * sizeof(uint32)) + ((yOffset + curX) * stride));
      __m128i bufferLo = _mm_loadu_si128((__m128i*)bufferPosition);
      __m128i bufferHi = _mm_loadu_si128(((__m128i*)bufferPosition) + 1);
      __m128i font = _mm_shuffle_epi8(fontRow, _mm_setzero_si128());
      fontRow = _mm_srli_si128(fontRow, 1);
      __m128i maskLo = _mm_and_si128(font, bitMaskLo);
      __m128i maskHi = _mm_and_si128(font, bitMaskHi);
      maskLo = _mm_sllv_epi32(maskLo, shiftMaskLo);
      maskHi = _mm_sllv_epi32(maskHi, shiftMaskHi);
      _mm_store_si128((__m128i*)bufferPosition, _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(bufferLo), _mm_castsi128_ps(colors), _mm_castsi128_ps(maskLo))));
      _mm_store_si128(((__m128i*)bufferPosition) + 1, _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(bufferHi), _mm_castsi128_ps(colors), _mm_castsi128_ps(maskHi))));
    }

    xOffset += 8;
    ++curChar;
  }
}

void Unaligned_DrawDebugText_AVX(const uint8 lut[128][8], const String& message, size_t x, size_t y, uint8* buffer, size_t width, const ZColor& color) {
  size_t xOffset = x;
  size_t yOffset = y;

  const char* curChar = message.Str();
  const size_t stride = width * sizeof(uint32);
  const uint32 colorValue = color.Color();

  __m256i bitMask = _mm256_set_epi32(1 << 7, 1 << 6, 1 << 5, 1 << 4, 1 << 3, 1 << 2, 1 << 1, 1);
  __m256i shiftMask = _mm256_set_epi32(24, 25, 26, 27, 28, 29, 30, 31);
  __m256i colors = _mm256_set1_epi32(colorValue);

  for (size_t strLen = 0; strLen < message.Length(); ++strLen) {
    const char fontChar = *curChar;
    __m128i fontRow = _mm_loadu_si64(lut[(uint8)fontChar]);
    for (size_t curX = 0; curX < 8; ++curX) {
      uint32* bufferPosition = (uint32*)(buffer + (xOffset * sizeof(uint32)) + ((yOffset + curX) * stride));
      __m256i font = _mm256_broadcastb_epi8(fontRow);
      __m256i mask = _mm256_and_si256(font, bitMask);
      mask = _mm256_sllv_epi32(mask, shiftMask);
      fontRow = _mm_srli_si128(fontRow, 1);
      _mm256_maskstore_epi32((int*)bufferPosition, mask, colors);
    }

    xOffset += 8;
    ++curChar;
  }
}

void Aligned_Mat4x4Transform(const float matrix[4][4], float* __restrict data, int32 stride, int32 length) {
  /*
    We preload 4 registers with all of the matrix X, Y, Z, W values.
    We also broadcast each vector component (X, Y, Z, W) to 4 registers.

    This saves us some arithmetic. We only have to perform 4 multiplies and 4 adds per vertex in the end.
  */

  __m128 matrixX = _mm_loadu_ps(matrix[0]);
  __m128 matrixY = _mm_loadu_ps(matrix[1]);
  __m128 matrixZ = _mm_loadu_ps(matrix[2]);
  __m128 matrixW = _mm_loadu_ps(matrix[3]);

  __m128 loXY = _mm_unpacklo_ps(matrixX, matrixY);
  __m128 loZW = _mm_unpacklo_ps(matrixZ, matrixW);
  __m128 hiXY = _mm_unpackhi_ps(matrixX, matrixY);
  __m128 hiZW = _mm_unpackhi_ps(matrixZ, matrixW);

  matrixX = _mm_shuffle_ps(loXY, loZW, 0b01000100);
  matrixY = _mm_shuffle_ps(loXY, loZW, 0b11101110);
  matrixZ = _mm_shuffle_ps(hiXY, hiZW, 0b01000100);
  matrixW = _mm_shuffle_ps(hiXY, hiZW, 0b11101110);

  for (int32 i = 0; i < length; i += stride) {
    float* __restrict vecData = data + i;

    __m128 xyzw = _mm_loadu_ps(vecData);

    __m128 vecX = _mm_shuffle_ps(xyzw, xyzw, 0b00000000);
    __m128 vecY = _mm_shuffle_ps(xyzw, xyzw, 0b01010101);
    __m128 vecZ = _mm_shuffle_ps(xyzw, xyzw, 0b10101010);
    __m128 vecW = _mm_shuffle_ps(xyzw, xyzw, 0b11111111);

    __m128 result = _mm_add_ps(_mm_mul_ps(matrixX, vecX), _mm_mul_ps(matrixY, vecY));
    result = _mm_add_ps(result, _mm_mul_ps(matrixZ, vecZ));
    result = _mm_add_ps(result, _mm_mul_ps(matrixW, vecW));

    _mm_storeu_ps(vecData, result);
  }
}

void Aligned_DepthBufferVisualize_SSE(float* buffer, size_t width, size_t height) {
  const float colorScaleValue = -255.f;

  __m128i initialColor = _mm_set1_epi32(0xFF000000);
  __m128 colorScale = _mm_set_ps1(colorScaleValue);

  __m128i shuffleControl = _mm_set_epi8(
    0x80U, 12, 12, 12,
    0x80U, 8, 8, 8,
    0x80U, 4, 4, 4,
    0x80U, 0, 0, 0
  );

  const size_t simdLength = (width >> 2) << 2;
  for (size_t h = 0; h < height; ++h) {
    float* currentDepth = (buffer + (h * width));
    size_t w = 0;
    for (; w < simdLength; w += 4, currentDepth += 4) {
      __m128 depthValues = _mm_loadu_ps(currentDepth);

      __m128i tempColors = _mm_cvtps_epi32(_mm_mul_ps(colorScale, depthValues));
      tempColors = _mm_shuffle_epi8(tempColors, shuffleControl);
      __m128i finalColor = _mm_or_si128(initialColor, tempColors);

      _mm_storeu_si128((__m128i*)currentDepth, finalColor);
    }

    for (; w < width; ++w, ++currentDepth) {
      float depth = *currentDepth;
      uint32 tempColor = (uint32)(colorScaleValue * depth);
      (*((uint32*)currentDepth)) = (0xFF000000 | (tempColor << 16) | (tempColor << 8) | tempColor);
    }
  }
}

void Aligned_DepthBufferVisualize_AVX(float* buffer, size_t width, size_t height) {
  const float colorScaleValue = -255.f;

  __m256i initialColor = _mm256_set1_epi32(0xFF000000);
  __m256 colorScale = _mm256_set1_ps(colorScaleValue);

  __m256i shuffleControl = _mm256_set_epi8(
    0x80U, 12, 12, 12,
    0x80U, 8, 8, 8,
    0x80U, 4, 4, 4,
    0x80U, 0, 0, 0,
    0x80U, 12, 12, 12,
    0x80U, 8, 8, 8,
    0x80U, 4, 4, 4,
    0x80U, 0, 0, 0
  );

  const size_t simdLength = (width >> 3) << 3;
  for (size_t h = 0; h < height; ++h) {
    float* currentDepth = (buffer + (h * width));
    size_t w = 0;
    for (; w < simdLength; w += 8, currentDepth += 8) {
      __m256 depthValues = _mm256_load_ps(currentDepth);

      __m256i tempColors = _mm256_cvtps_epi32(_mm256_mul_ps(colorScale, depthValues));
      tempColors = _mm256_shuffle_epi8(tempColors, shuffleControl);
      __m256i finalColor = _mm256_or_si256(initialColor, tempColors);

      _mm256_storeu_si256((__m256i*)currentDepth, finalColor);
    }

    for (; w < width; ++w, ++currentDepth) {
      float depth = *currentDepth;
      uint32 tempColor = (uint32)(colorScaleValue * depth);
      (*((uint32*)currentDepth)) = (0xFF000000 | (tempColor << 16) | (tempColor << 8) | tempColor);
    }
  }
}

void Aligned_Vec4Homogenize(float* data, int32 stride, int32 length) {
  for (int32 i = 0; i < length; i += stride) {
    float* nextVec = data + i;
    __m128 vec = _mm_loadu_ps(nextVec);
    __m128 perspectiveTerm = _mm_shuffle_ps(vec, vec, 0b11111111);
    __m128 result = _mm_mul_ps(vec, _mm_rcp_ps(perspectiveTerm));
    result = _mm_blend_ps(result, perspectiveTerm, 0b1000);
    _mm_storeu_ps(nextVec, result);
  }
}

void Unaligned_BlendBuffers_SSE(uint32* __restrict devBuffer, uint32* __restrict frameBuffer, size_t width, size_t height, float opacity) {
  short devOpacityValue = (short)(255.f * opacity);
  short bufferOpacityValue = (short)(255.f * (1.f - opacity));
  
  __m128i devOpacity = _mm_set1_epi16(devOpacityValue << 8);
  __m128i bufferOpacity = _mm_set1_epi16(bufferOpacityValue << 8);

  const size_t simdLength = (width >> 2) << 2;
  for (size_t y = 0; y < height; ++y) {
    size_t x = 0;
    for (; x < simdLength; x += 4) {
      size_t index = (y * width) + x;

      // Load 4 pixels at a time
      // Split the blend function into two halves because SSE doesn't have artihmetic insns for 8bit values
      // The best we can do is 16bit arithmetic
      __m128i devColor = _mm_loadu_si128((__m128i*)(devBuffer + index));
      __m128i bufferColor = _mm_loadu_si128((__m128i*)(frameBuffer + index));

      __m128i devLo16 = _mm_unpacklo_epi8(devColor, _mm_setzero_si128()); // 2x 32bit BGRA, [0,1]
      __m128i devHi16 = _mm_unpackhi_epi8(devColor, _mm_setzero_si128()); // 2x 32bit BGRA, [2,3]

      __m128i bufLo16 = _mm_unpacklo_epi8(bufferColor, _mm_setzero_si128());
      __m128i bufHi16 = _mm_unpackhi_epi8(bufferColor, _mm_setzero_si128());

      devLo16 = _mm_mulhi_epu16(devLo16, devOpacity);
      devHi16 = _mm_mulhi_epu16(devHi16, devOpacity);

      bufLo16 = _mm_mulhi_epu16(bufLo16, bufferOpacity);
      bufHi16 = _mm_mulhi_epu16(bufHi16, bufferOpacity);

      __m128i loResult = _mm_add_epi16(devLo16, bufLo16);
      __m128i hiResult = _mm_add_epi16(devHi16, bufHi16);

      // Pack the 16bit results into 8bit values and store it back to memory
      _mm_storeu_si128((__m128i*)(frameBuffer + index), _mm_packus_epi16(loResult, hiResult));
    }

    for (; x < width; ++x) {
      size_t index = (y * width) + x;
      uint8* __restrict devPixels = (uint8* __restrict)(devBuffer + index);
      uint8* __restrict bufPixels = (uint8* __restrict)(frameBuffer + index);

      short devB = devPixels[0];
      short devG = devPixels[1];
      short devR = devPixels[2];
      short devA = devPixels[3];

      short bufB = bufPixels[0];
      short bufG = bufPixels[1];
      short bufR = bufPixels[2];
      short bufA = bufPixels[3];

      devB *= devOpacityValue;
      devG *= devOpacityValue;
      devR *= devOpacityValue;
      devA *= devOpacityValue;
      devB >>= 8;
      devG >>= 8;
      devR >>= 8;
      devA >>= 8;

      bufB *= bufferOpacityValue;
      bufG *= bufferOpacityValue;
      bufR *= bufferOpacityValue;
      bufA *= bufferOpacityValue;
      bufB >>= 8;
      bufG >>= 8;
      bufR >>= 8;
      bufA >>= 8;

      bufPixels[0] = (uint8)(bufB + devB);
      bufPixels[1] = (uint8)(bufG + devG);
      bufPixels[2] = (uint8)(bufR + devR);
      bufPixels[3] = (uint8)(bufA + devA);
    }
  }
}

void Unaligned_BlendBuffers_AVX(uint32* __restrict devBuffer, uint32* __restrict frameBuffer, size_t width, size_t height, float opacity) {
  short devOpacityValue = (short)(255.f * opacity);
  short bufferOpacityValue = (short)(255.f * (1.f - opacity));

  __m256i devOpacity = _mm256_set1_epi16(devOpacityValue << 8);
  __m256i bufferOpacity = _mm256_set1_epi16(bufferOpacityValue << 8);

  const size_t simdLength = (width >> 3) << 3;
  for (size_t y = 0; y < height; ++y) {
    size_t x = 0;
    for (; x < simdLength; x += 8) {
      size_t index = (y * width) + x;

      // Load 8 pixels at a time
      // Split the blend function into two halves because SSE doesn't have artihmetic insns for 8bit values
      // The best we can do is 16bit arithmetic
      __m256i devColor = _mm256_loadu_si256((__m256i*)(devBuffer + index));
      __m256i bufferColor = _mm256_loadu_si256((__m256i*)(frameBuffer + index));

      __m256i devLo16 = _mm256_unpacklo_epi8(devColor, _mm256_setzero_si256()); // 4x 32bit BGRA, [0,1]
      __m256i devHi16 = _mm256_unpackhi_epi8(devColor, _mm256_setzero_si256()); // 4x 32bit BGRA, [2,3]

      __m256i bufLo16 = _mm256_unpacklo_epi8(bufferColor, _mm256_setzero_si256());
      __m256i bufHi16 = _mm256_unpackhi_epi8(bufferColor, _mm256_setzero_si256());

      devLo16 = _mm256_mulhi_epu16(devLo16, devOpacity);
      devHi16 = _mm256_mulhi_epu16(devHi16, devOpacity);

      bufLo16 = _mm256_mulhi_epu16(bufLo16, bufferOpacity);
      bufHi16 = _mm256_mulhi_epu16(bufHi16, bufferOpacity);

      __m256i loResult = _mm256_add_epi16(devLo16, bufLo16);
      __m256i hiResult = _mm256_add_epi16(devHi16, bufHi16);

      // Pack the 16bit results into 8bit values and store it back to memory
      _mm256_storeu_si256((__m256i*)(frameBuffer + index), _mm256_packus_epi16(loResult, hiResult));
    }

    for (; x < width; ++x) {
      size_t index = (y * width) + x;
      uint8* __restrict devPixels = (uint8 * __restrict)(devBuffer + index);
      uint8* __restrict bufPixels = (uint8 * __restrict)(frameBuffer + index);

      short devB = devPixels[0];
      short devG = devPixels[1];
      short devR = devPixels[2];
      short devA = devPixels[3];

      short bufB = bufPixels[0];
      short bufG = bufPixels[1];
      short bufR = bufPixels[2];
      short bufA = bufPixels[3];

      devB *= devOpacityValue;
      devG *= devOpacityValue;
      devR *= devOpacityValue;
      devA *= devOpacityValue;
      devB >>= 8;
      devG >>= 8;
      devR >>= 8;
      devA >>= 8;

      bufB *= bufferOpacityValue;
      bufG *= bufferOpacityValue;
      bufR *= bufferOpacityValue;
      bufA *= bufferOpacityValue;
      bufB >>= 8;
      bufG >>= 8;
      bufR >>= 8;
      bufA >>= 8;

      bufPixels[0] = (uint8)(bufB + devB);
      bufPixels[1] = (uint8)(bufG + devG);
      bufPixels[2] = (uint8)(bufR + devR);
      bufPixels[3] = (uint8)(bufA + devA);
    }
  }
}

void Aligned_BackfaceCull(IndexBuffer& indexBuffer, const VertexBuffer& vertexBuffer) {
  /*
    NOTE: We're performing backface culling in camera space, post-perspective transform.
  */

  int32* indexData = indexBuffer.GetInputData();
  const float* vertexData = vertexBuffer[0];

  int32* endAddr = indexData + (indexBuffer.GetIndexSize() - 3);

  __m128i dotSign = _mm_set_epi32(0, 0x80000000, 0, 0);

  for (int32 i = indexBuffer.GetIndexSize(); i > 0; i -= 3) {
    int32* currentData = indexData + i - 3;
    __m128 v1 = _mm_loadu_ps(vertexData + currentData[0]);
    __m128 v2 = _mm_loadu_ps(vertexData + currentData[1]);
    __m128 v3 = _mm_loadu_ps(vertexData + currentData[2]);

    __m128 p1p0 = _mm_sub_ps(v2, v1);
    __m128 p2p0 = _mm_sub_ps(v3, v1);

    __m128 normal = Cross128(p1p0, p2p0);

    if (_mm_testc_si128(_mm_castps_si128(normal), dotSign)) {
      ((int64*)currentData)[0] = ((int64*)endAddr)[0];
      currentData[2] = endAddr[2];
      endAddr -= 3;
    }
  }

  int32 resultingSize = (int32)(endAddr + 3 - indexData);
  indexBuffer.SetIndexSize(resultingSize);
}

void Aligned_WindowTransform(float* data, int32 stride, int32 length, const float windowTransform0[3], const float windowTransform1[3], const float width, const float height) {
  __m128 window0 = _mm_set_ps(0.f, windowTransform0[2], windowTransform0[1], windowTransform0[0]);
  __m128 window1 = _mm_set_ps(0.f, windowTransform1[2], windowTransform1[1], windowTransform1[0]);

  __m128 maxXY = _mm_set_ps(0.f, 0.f, height, width);

  for (int32 i = 0; i < length; i += stride) {
    float* vertexData = data + i;

    __m128 vec = _mm_loadu_ps(vertexData);
    __m128 invPerspectiveZ = _mm_rcp_ps(_mm_shuffle_ps(vec, vec, 0b11111111));
    __m128 invDivisor = _mm_rcp_ps(_mm_shuffle_ps(vec, vec, 0b10101010));

    // Homogenize with Z
    __m128 result = _mm_mul_ps(vec, invDivisor);

    // Apply Window transform.
    __m128 dotX = _mm_mul_ps(result, window0);
    __m128 dotY = _mm_mul_ps(result, window1);

    result = _mm_hadd_ps(_mm_hadd_ps(dotX, dotY), _mm_setzero_ps());
    result = _mm_min_ps(_mm_max_ps(result, _mm_setzero_ps()), maxXY);

    // [0] = dotX
    // [1] = dotY
    // [2] = _
    // [3] = invPerspectiveZ
    result = _mm_blend_ps(result, invPerspectiveZ, 0b1000);

    _mm_storeu_ps(vertexData, result);

    for (int32 j = 4; j < stride; j += 4) {
      _mm_storeu_ps(vertexData + j, _mm_mul_ps(_mm_loadu_ps(vertexData + j), invPerspectiveZ));
    }
  }
}

void Aligned_TransformDirectScreenSpace(float* data, int32 stride, int32 length, const float matrix[4][4], const float windowTransform0[3], const float windowTransform1[3], const float width, const float height) {
  __m128 matrixX = _mm_loadu_ps(matrix[0]);
  __m128 matrixY = _mm_loadu_ps(matrix[1]);
  __m128 matrixZ = _mm_loadu_ps(matrix[2]);
  __m128 matrixW = _mm_loadu_ps(matrix[3]);

  __m128 loXY = _mm_unpacklo_ps(matrixX, matrixY);
  __m128 loZW = _mm_unpacklo_ps(matrixZ, matrixW);
  __m128 hiXY = _mm_unpackhi_ps(matrixX, matrixY);
  __m128 hiZW = _mm_unpackhi_ps(matrixZ, matrixW);

  matrixX = _mm_shuffle_ps(loXY, loZW, 0b01000100);
  matrixY = _mm_shuffle_ps(loXY, loZW, 0b11101110);
  matrixZ = _mm_shuffle_ps(hiXY, hiZW, 0b01000100);
  matrixW = _mm_shuffle_ps(hiXY, hiZW, 0b11101110);

  __m128 window0 = _mm_set_ps(0.f, windowTransform0[2], windowTransform0[1], windowTransform0[0]);
  __m128 window1 = _mm_set_ps(0.f, windowTransform1[2], windowTransform1[1], windowTransform1[0]);

  __m128 maxXY = _mm_set_ps(0.f, 0.f, height, width);

  for (int32 i = 0; i < length; i += stride) {
    float* vecData = data + i;

    // Perspective projection
    __m128 xyzw = _mm_loadu_ps(vecData);

    __m128 vecX = _mm_shuffle_ps(xyzw, xyzw, 0b00000000);
    __m128 vecY = _mm_shuffle_ps(xyzw, xyzw, 0b01010101);
    __m128 vecZ = _mm_shuffle_ps(xyzw, xyzw, 0b10101010);
    __m128 vecW = _mm_shuffle_ps(xyzw, xyzw, 0b11111111);

    __m128 vec = _mm_add_ps(_mm_mul_ps(matrixX, vecX), _mm_mul_ps(matrixY, vecY));
    vec = _mm_add_ps(vec, _mm_mul_ps(matrixZ, vecZ));
    vec = _mm_add_ps(vec, _mm_mul_ps(matrixW, vecW));

    // Homogenize
    __m128 invPerspectiveZ = _mm_rcp_ps(_mm_shuffle_ps(vec, vec, 0b11111111));
    __m128 homogenized = _mm_mul_ps(vec, invPerspectiveZ);

    // Window transform
    __m128 invDivisor = _mm_rcp_ps(_mm_shuffle_ps(homogenized, homogenized, 0b10101010));

    // Homogenize with Z
    __m128 result = _mm_mul_ps(homogenized, invDivisor);

    // Apply Window transform.
    __m128 dotX = _mm_mul_ps(result, window0);
    __m128 dotY = _mm_mul_ps(result, window1);

    result = _mm_hadd_ps(_mm_hadd_ps(dotX, dotY), _mm_setzero_ps());
    result = _mm_min_ps(_mm_max_ps(result, _mm_setzero_ps()), maxXY);

    // [0] = dotX
    // [1] = dotY
    // [2] = _
    // [3] = invPerspectiveZ
    result = _mm_blend_ps(result, invPerspectiveZ, 0b1000);

    _mm_storeu_ps(vecData, result);

    for (int32 j = 4; j < stride; j+=4) {
      _mm_storeu_ps(vecData + j, _mm_mul_ps(_mm_loadu_ps(vecData + j), invPerspectiveZ));
    }
  }
}

void Aligned_HomogenizeTransformScreenSpace(float* data, int32 stride, int32 length, const float windowTransform0[3], const float windowTransform1[3], const float width, const float height) {
  __m128 window0 = _mm_set_ps(0.f, windowTransform0[2], windowTransform0[1], windowTransform0[0]);
  __m128 window1 = _mm_set_ps(0.f, windowTransform1[2], windowTransform1[1], windowTransform1[0]);

  __m128 maxXY = _mm_set_ps(0.f, 0.f, height, width);
  
  for (int32 i = 0; i < length; i += stride) {
    float* nextVec = data + i;
    __m128 vec = _mm_loadu_ps(nextVec);
    __m128 invPerspectiveZ = _mm_rcp_ps(_mm_shuffle_ps(vec, vec, 0b11111111));
    __m128 result = _mm_mul_ps(vec, invPerspectiveZ);

    __m128 invDivisor = _mm_rcp_ps(_mm_shuffle_ps(result, result, 0b10101010));

    // Homogenize with Z
    result = _mm_mul_ps(result, invDivisor);

    // Apply Window transform.
    __m128 dotX = _mm_mul_ps(result, window0);
    __m128 dotY = _mm_mul_ps(result, window1);

    result = _mm_hadd_ps(_mm_hadd_ps(dotX, dotY), _mm_setzero_ps());
    result = _mm_min_ps(_mm_max_ps(result, _mm_setzero_ps()), maxXY);

    // [0] = dotX
    // [1] = dotY
    // [2] = _
    // [3] = invPerspectiveZ
    result = _mm_blend_ps(result, invPerspectiveZ, 0b1000);

    _mm_storeu_ps(nextVec, result);

    for (int32 j = 4; j < stride; j += 4) {
      _mm_storeu_ps(nextVec + j, _mm_mul_ps(_mm_loadu_ps(nextVec + j), invPerspectiveZ));
    }
  }
}

void Unaligned_AABB_SSE(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]) {
  __m128 min = _mm_loadu_ps(outMin);
  __m128 max = _mm_loadu_ps(outMax);

  for (size_t i = 0; i < numVertices; i += stride) {
    __m128 vertex = _mm_loadu_ps(vertices + i);
    min = _mm_min_ps(min, vertex);
    max = _mm_max_ps(max, vertex);
  }

  _mm_storeu_ps(outMin, min);
  _mm_storeu_ps(outMax, max);
}

void Unaligned_AABB_AVX(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]) {
  bool packedVerts = stride == 4;

  if (packedVerts) {
    __m128 min = _mm_loadu_ps(outMin);
    __m128 max = _mm_loadu_ps(outMax);

    __m256 wideMin = _mm256_set_m128(min, min);
    __m256 wideMax = _mm256_set_m128(max, max);

    size_t i = 0;
    size_t simdLength = (numVertices >> 3) << 3;
    for (; i < simdLength; i += 8) {
      __m256 vertex = _mm256_loadu_ps(vertices + i);
      wideMin = _mm256_min_ps(wideMin, vertex);
      wideMax = _mm256_max_ps(wideMax, vertex);
    }

    // If we reach the end, check for the final vertex.
    if (i < numVertices) {
      __m128 vertex = _mm_loadu_ps(vertices + i);
      min = _mm_min_ps(min, vertex);
      max = _mm_max_ps(max, vertex);
    }

    // Reduce the hi/lo vectors
    min = _mm_min_ps(_mm256_castps256_ps128(wideMin), min);
    min = _mm_min_ps(_mm256_extractf128_ps(wideMin, 0b1), min);

    max = _mm_max_ps(_mm256_castps256_ps128(wideMax), max);
    max = _mm_max_ps(_mm256_extractf128_ps(wideMax, 0b1), max);

    _mm_storeu_ps(outMin, min);
    _mm_storeu_ps(outMax, max);
  }
  else {
    Unaligned_AABB_SSE(vertices, numVertices, stride, outMin, outMax);
  }
}

void Unaligned_Shader_RGB_SSE(const float* __restrict vertices, const int32* __restrict indices, const int32 end, const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer) {
  const int32 sMaxWidth = (int32)maxWidth;

  __m128 initMultiplier = _mm_set_ps(3.f, 2.f, 1.f, 0.f);
  __m128 stepMultiplier = _mm_set_ps1(4.f);
  __m128i initialColor = _mm_set1_epi32(0xFF000000);
  __m128 rgbScale = _mm_set_ps1(255.f);
  __m128i weightMask = _mm_set1_epi32(0x80000000);

  for (int32 i = 0; i < end; i += 3) {
    const float* __restrict v1 = vertices + indices[i];
    const float* __restrict v2 = vertices + indices[i + 1];
    const float* __restrict v3 = vertices + indices[i + 2];

    __m128 v1All = _mm_loadu_ps(v1);
    __m128 v2All = _mm_loadu_ps(v2);
    __m128 v3All = _mm_loadu_ps(v3);
    __m128 v1Attrs = _mm_loadu_ps(v1 + 4);
    __m128 v2Attrs = _mm_loadu_ps(v2 + 4);
    __m128 v3Attrs = _mm_loadu_ps(v3 + 4);

    __m128 x0 = _mm_shuffle_ps(v1All, v1All, 0b00000000);
    __m128 x1 = _mm_shuffle_ps(v2All, v2All, 0b00000000);
    __m128 x2 = _mm_shuffle_ps(v3All, v3All, 0b00000000);

    __m128 y0 = _mm_shuffle_ps(v1All, v1All, 0b01010101);
    __m128 y1 = _mm_shuffle_ps(v2All, v2All, 0b01010101);
    __m128 y2 = _mm_shuffle_ps(v3All, v3All, 0b01010101);

    __m128 fmins = _mm_min_ps(_mm_min_ps(v1All, v2All), v3All);
    __m128 fmaxs = _mm_max_ps(_mm_max_ps(v1All, v2All), v3All);

    fmins = _mm_round_ps(fmins, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
    fmaxs = _mm_round_ps(fmaxs, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);

    __m128 fminX = _mm_shuffle_ps(fmins, fmins, 0b00000000);
    __m128 fminY = _mm_shuffle_ps(fmins, fmins, 0b01010101);

    __m128i imins = _mm_cvtps_epi32(fmins);
    __m128i imaxs = _mm_cvtps_epi32(fmaxs);

    long long minXY = _mm_cvtsi128_si64(imins);
    long long maxXY = _mm_cvtsi128_si64(imaxs);

    int32 minX = (int32)minXY;
    int32 minY = minXY >> 32;

    int32 maxX = (int32)maxXY;
    int32 maxY = maxXY >> 32;

    __m128 x1x0 = _mm_sub_ps(x1, x0);
    __m128 x2x1 = _mm_sub_ps(x2, x1);
    __m128 x0x2 = _mm_sub_ps(x0, x2);
    __m128 y1y0 = _mm_sub_ps(y1, y0);
    __m128 y2y0 = _mm_sub_ps(y2, y0);

    __m128 invArea = _mm_rcp_ps(_mm_sub_ps(_mm_mul_ps(_mm_sub_ps(x2, x0), y1y0), _mm_mul_ps(y2y0, x1x0)));

    __m128 z0 = _mm_shuffle_ps(v1All, v1All, 0b11111111);
    __m128 invVert0 = _mm_mul_ps(z0, invArea);
    __m128 invVert1 = _mm_mul_ps(_mm_shuffle_ps(v2All, v2All, 0b11111111), invArea);
    __m128 invVert2 = _mm_mul_ps(_mm_shuffle_ps(v3All, v3All, 0b11111111), invArea);

    __m128 z1z0 = _mm_sub_ps(invVert1, invVert0);
    __m128 z2z0 = _mm_sub_ps(invVert2, invVert0);

    // We want the RGB values to be scaled by 255 in the end.
    // Doing that here saves us from having to apply the scale at each pixel.
    __m128 scaleFactor = _mm_mul_ps(invArea, rgbScale);

    __m128 rgbScaledV1 = _mm_mul_ps(v1Attrs, rgbScale);

    __m128 r0 = _mm_shuffle_ps(rgbScaledV1, rgbScaledV1, 0b00000000);
    __m128 g0 = _mm_shuffle_ps(rgbScaledV1, rgbScaledV1, 0b01010101);
    __m128 b0 = _mm_shuffle_ps(rgbScaledV1, rgbScaledV1, 0b10101010);

    __m128 scaledV1 = _mm_mul_ps(v1Attrs, scaleFactor);
    __m128 scaledV2 = _mm_mul_ps(v2Attrs, scaleFactor);
    __m128 scaledV3 = _mm_mul_ps(v3Attrs, scaleFactor);

    __m128 invAttr00 = _mm_shuffle_ps(scaledV1, scaledV1, 0b00000000);
    __m128 invAttr01 = _mm_shuffle_ps(scaledV2, scaledV2, 0b00000000);
    __m128 invAttr02 = _mm_shuffle_ps(scaledV3, scaledV3, 0b00000000);

    __m128 r1r0 = _mm_sub_ps(invAttr01, invAttr00);
    __m128 r2r0 = _mm_sub_ps(invAttr02, invAttr00);

    __m128 invAttr10 = _mm_shuffle_ps(scaledV1, scaledV1, 0b01010101);
    __m128 invAttr11 = _mm_shuffle_ps(scaledV2, scaledV2, 0b01010101);
    __m128 invAttr12 = _mm_shuffle_ps(scaledV3, scaledV3, 0b01010101);

    __m128 g1g0 = _mm_sub_ps(invAttr11, invAttr10);
    __m128 g2g0 = _mm_sub_ps(invAttr12, invAttr10);

    __m128 invAttr20 = _mm_shuffle_ps(scaledV1, scaledV1, 0b10101010);
    __m128 invAttr21 = _mm_shuffle_ps(scaledV2, scaledV2, 0b10101010);
    __m128 invAttr22 = _mm_shuffle_ps(scaledV3, scaledV3, 0b10101010);

    __m128 b1b0 = _mm_sub_ps(invAttr21, invAttr20);
    __m128 b2b0 = _mm_sub_ps(invAttr22, invAttr20);

    // Calculate the step amount for each horizontal and vertical pixel out of the main loop.
    /*
      float boundingBoxMin[2] = {fminX, fminY};
      __m256 weightInit0 = _mm256_set1_ps(BarycentricArea2D(v2, v3, boundingBoxMin));
      __m256 weightInit1 = _mm256_set1_ps(BarycentricArea2D(v3, v1, boundingBoxMin));
      __m256 weightInit2 = _mm256_set1_ps(BarycentricArea2D(v1, v2, boundingBoxMin));
    */
    __m128 weightInit0 = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(fminX, x1), _mm_sub_ps(y2, y1)), _mm_mul_ps(_mm_sub_ps(fminY, y1), x2x1));
    __m128 weightInit1 = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(fminX, x2), _mm_sub_ps(y0, y2)), _mm_mul_ps(_mm_sub_ps(fminY, y2), x0x2));
    __m128 weightInit2 = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(fminX, x0), y1y0), _mm_mul_ps(_mm_sub_ps(fminY, y0), x1x0));

    __m128 xStep0 = _mm_sub_ps(y1, y2);
    __m128 xStep1 = y2y0;
    __m128 xStep2 = _mm_sub_ps(y0, y1);

    __m128 yStep0 = x2x1;
    __m128 yStep1 = x0x2;
    __m128 yStep2 = x1x0;

    weightInit0 = _mm_sub_ps(weightInit0, _mm_mul_ps(initMultiplier, xStep0));
    weightInit1 = _mm_sub_ps(weightInit1, _mm_mul_ps(initMultiplier, xStep1));
    weightInit2 = _mm_sub_ps(weightInit2, _mm_mul_ps(initMultiplier, xStep2));

    xStep0 = _mm_mul_ps(stepMultiplier, xStep0);
    xStep1 = _mm_mul_ps(stepMultiplier, xStep1);
    xStep2 = _mm_mul_ps(stepMultiplier, xStep2);

    for (int32 h = minY; h < maxY; ++h) {
      __m128 weights0 = weightInit0;
      __m128 weights1 = weightInit1;
      __m128 weights2 = weightInit2;

      uint32* __restrict pixels = ((uint32* __restrict)(framebuffer)) + (minX + (h * sMaxWidth));
      float* __restrict pixelDepth = depthBuffer + (minX + (h * sMaxWidth));

      for (int32 w = minX; w < maxX; w += 4, pixels += 4, pixelDepth += 4) {
        // Fetch all sign bits and OR them together
        __m128 combinedWeights = _mm_or_ps(_mm_or_ps(weights0, weights1), weights2);

        // If all mask bits are set then none of these pixels are inside the triangle.
        if (!_mm_testc_si128(_mm_castps_si128(combinedWeights), weightMask)) {
          // Our 4-wide alignment doesn't match at the moment. Possibly revisit in the future.
          __m128i pixelVec = _mm_loadu_si128((__m128i* __restrict)pixels);
          __m128 depthVec = _mm_loadu_ps(pixelDepth);

          __m128 weightedVerts0 = z0;
          __m128 weightedVerts1 = _mm_mul_ps(weights1, z1z0);
          __m128 weightedVerts2 = _mm_mul_ps(weights2, z2z0);

          __m128 zValues = _mm_rcp_ps(_mm_add_ps(_mm_add_ps(weightedVerts0, weightedVerts1), weightedVerts2));

          __m128 depthMask = _mm_cmp_ps(zValues, depthVec, _CMP_LT_OQ);

          __m128i finalCombinedMask = _mm_castps_si128(_mm_or_ps(combinedWeights, depthMask));

          __m128 weightedAttr00 = _mm_mul_ps(r0, zValues);
          __m128 weightedAttr01 = _mm_mul_ps(_mm_mul_ps(weights1, r1r0), zValues);
          __m128 weightedAttr02 = _mm_mul_ps(_mm_mul_ps(weights2, r2r0), zValues);

          __m128 weightedAttr10 = _mm_mul_ps(g0, zValues);
          __m128 weightedAttr11 = _mm_mul_ps(_mm_mul_ps(weights1, g1g0), zValues);
          __m128 weightedAttr12 = _mm_mul_ps(_mm_mul_ps(weights2, g2g0), zValues);

          __m128 weightedAttr20 = _mm_mul_ps(b0, zValues);
          __m128 weightedAttr21 = _mm_mul_ps(_mm_mul_ps(weights1, b1b0), zValues);
          __m128 weightedAttr22 = _mm_mul_ps(_mm_mul_ps(weights2, b2b0), zValues);

          __m128i rValues = _mm_cvtps_epi32(_mm_add_ps(_mm_add_ps(weightedAttr00, weightedAttr01), weightedAttr02));
          __m128i gValues = _mm_cvtps_epi32(_mm_add_ps(_mm_add_ps(weightedAttr10, weightedAttr11), weightedAttr12));
          __m128i bValues = _mm_cvtps_epi32(_mm_add_ps(_mm_add_ps(weightedAttr20, weightedAttr21), weightedAttr22));

          rValues = _mm_slli_epi32(rValues, 16);
          gValues = _mm_slli_epi32(gValues, 8);

          __m128i finalColor = initialColor;
          finalColor = _mm_or_si128(finalColor, rValues);
          finalColor = _mm_or_si128(finalColor, gValues);
          finalColor = _mm_or_si128(finalColor, bValues);

          __m128i writebackColor = _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(finalColor), _mm_castsi128_ps(pixelVec), _mm_castsi128_ps(finalCombinedMask)));
          __m128 writebackDepth = _mm_blendv_ps(zValues, depthVec, _mm_castsi128_ps(finalCombinedMask));

          _mm_storeu_si128((__m128i* __restrict)pixels, writebackColor);
          _mm_storeu_ps(pixelDepth, writebackDepth);
        }

        weights0 = _mm_sub_ps(weights0, xStep0);
        weights1 = _mm_sub_ps(weights1, xStep1);
        weights2 = _mm_sub_ps(weights2, xStep2);
      }

      weightInit0 = _mm_sub_ps(weightInit0, yStep0);
      weightInit1 = _mm_sub_ps(weightInit1, yStep1);
      weightInit2 = _mm_sub_ps(weightInit2, yStep2);
    }
  }
}

void Unaligned_Shader_RGB_AVX(const float* __restrict vertices, const int32* __restrict indices, const int32 end, const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer) {
  const int32 sMaxWidth = (int32)maxWidth;

  __m256 rgbScale = _mm256_set1_ps(255.f);
  __m256 initMultiplier = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
  __m256 stepMultiplier = _mm256_set1_ps(8.f);
  __m256i initialColor = _mm256_set1_epi32(0xFF000000);
  // Generating values on the fly is almost never worth it on x86.
  // It's much better to just take the extra memory op. This is specifically called out in the optimization manuals.
  __m256 signMask = _mm256_set1_ps(-0.f);

  __m256i yShuffle = _mm256_set1_epi32(1);
  __m256i zShuffle = _mm256_set1_epi32(3);
  __m256i rShuffle = _mm256_set1_epi32(4);
  __m256i gShuffle = _mm256_set1_epi32(5);
  __m256i bShuffle = _mm256_set1_epi32(6);

  for (int32 i = 0; i < end; i += 3) {
    __m256 v1All = _mm256_loadu_ps(vertices + indices[i]);
    __m256 v2All = _mm256_loadu_ps(vertices + indices[i + 1]);
    __m256 v3All = _mm256_loadu_ps(vertices + indices[i + 2]);

    __m256 x0 = _mm256_broadcastss_ps(_mm256_castps256_ps128(v1All));
    __m256 x1 = _mm256_broadcastss_ps(_mm256_castps256_ps128(v2All));
    __m256 x2 = _mm256_broadcastss_ps(_mm256_castps256_ps128(v3All));

    __m256 y0 = _mm256_permutevar8x32_ps(v1All, yShuffle);
    __m256 y1 = _mm256_permutevar8x32_ps(v2All, yShuffle);
    __m256 y2 = _mm256_permutevar8x32_ps(v3All, yShuffle);

    __m128 fmins = _mm_min_ps(_mm_min_ps(_mm256_castps256_ps128(v1All), _mm256_castps256_ps128(v2All)), _mm256_castps256_ps128(v3All));
    __m128 fmaxs = _mm_max_ps(_mm_max_ps(_mm256_castps256_ps128(v1All), _mm256_castps256_ps128(v2All)), _mm256_castps256_ps128(v3All));

    fmins = _mm_round_ps(fmins, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
    fmaxs = _mm_round_ps(fmaxs, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);

    __m256 fminX = _mm256_broadcastss_ps(fmins);
    __m256 fminY = _mm256_permutevar8x32_ps(_mm256_castps128_ps256(fmins), yShuffle);

    __m128i imins = _mm_cvtps_epi32(fmins);
    __m128i imaxs = _mm_cvtps_epi32(fmaxs);

    long long minXY = _mm_cvtsi128_si64(imins);
    long long maxXY = _mm_cvtsi128_si64(imaxs);

    int32 minX = (int32)minXY;
    int32 minY = minXY >> 32;

    int32 maxX = (int32)maxXY;
    int32 maxY = maxXY >> 32;

    __m256 x1x0 = _mm256_sub_ps(x1, x0);
    __m256 x2x1 = _mm256_sub_ps(x2, x1);
    __m256 x0x2 = _mm256_sub_ps(x0, x2);
    __m256 y1y0 = _mm256_sub_ps(y1, y0);
    __m256 y2y0 = _mm256_sub_ps(y2, y0);

    __m256 invArea = _mm256_rcp_ps(_mm256_fmsub_ps(_mm256_sub_ps(x2, x0), y1y0, _mm256_mul_ps(y2y0, x1x0)));

    __m256 z0 = _mm256_permutevar8x32_ps(v1All, zShuffle);

    __m256 invVert0 = _mm256_mul_ps(z0, invArea);
    __m256 invVert1 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v2All, zShuffle), invArea);
    __m256 invVert2 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v3All, zShuffle), invArea);

    __m256 z1z0 = _mm256_sub_ps(invVert1, invVert0);
    __m256 z2z0 = _mm256_sub_ps(invVert2, invVert0);

    // We want the RGB values to be scaled by 255 in the end.
    // Doing that here saves us from having to apply the scale at each pixel.
    __m256 scaleFactor = _mm256_mul_ps(invArea, rgbScale);

    __m256 rgbScaledV1 = _mm256_mul_ps(v1All, rgbScale);

    __m256 r0 = _mm256_permutevar8x32_ps(rgbScaledV1, rShuffle);
    __m256 g0 = _mm256_permutevar8x32_ps(rgbScaledV1, gShuffle);
    __m256 b0 = _mm256_permutevar8x32_ps(rgbScaledV1, bShuffle);

    __m256 scaledV1 = _mm256_mul_ps(v1All, scaleFactor);
    __m256 scaledV2 = _mm256_mul_ps(v2All, scaleFactor);
    __m256 scaledV3 = _mm256_mul_ps(v3All, scaleFactor);

    __m256 invAttr00 = _mm256_permutevar8x32_ps(scaledV1, rShuffle);
    __m256 invAttr01 = _mm256_permutevar8x32_ps(scaledV2, rShuffle);
    __m256 invAttr02 = _mm256_permutevar8x32_ps(scaledV3, rShuffle);

    __m256 r1r0 = _mm256_sub_ps(invAttr01, invAttr00);
    __m256 r2r0 = _mm256_sub_ps(invAttr02, invAttr00);

    __m256 invAttr10 = _mm256_permutevar8x32_ps(scaledV1, gShuffle);
    __m256 invAttr11 = _mm256_permutevar8x32_ps(scaledV2, gShuffle);
    __m256 invAttr12 = _mm256_permutevar8x32_ps(scaledV3, gShuffle);

    __m256 g1g0 = _mm256_sub_ps(invAttr11, invAttr10);
    __m256 g2g0 = _mm256_sub_ps(invAttr12, invAttr10);

    __m256 invAttr20 = _mm256_permutevar8x32_ps(scaledV1, bShuffle);
    __m256 invAttr21 = _mm256_permutevar8x32_ps(scaledV2, bShuffle);
    __m256 invAttr22 = _mm256_permutevar8x32_ps(scaledV3, bShuffle);

    __m256 b1b0 = _mm256_sub_ps(invAttr21, invAttr20);
    __m256 b2b0 = _mm256_sub_ps(invAttr22, invAttr20);

    // Calculate the step amount for each horizontal and vertical pixel out of the main loop.
    __m256 weightInit0 = _mm256_fmsub_ps(_mm256_sub_ps(fminX, x1), _mm256_sub_ps(y2, y1), _mm256_mul_ps(_mm256_sub_ps(fminY, y1), x2x1));
    __m256 weightInit1 = _mm256_fmsub_ps(_mm256_sub_ps(fminX, x2), _mm256_sub_ps(y0, y2), _mm256_mul_ps(_mm256_sub_ps(fminY, y2), x0x2));
    __m256 weightInit2 = _mm256_fmsub_ps(_mm256_sub_ps(fminX, x0), y1y0, _mm256_mul_ps(_mm256_sub_ps(fminY, y0), x1x0));

    __m256 xStep0 = _mm256_sub_ps(y1, y2);
    __m256 xStep1 = y2y0;
    __m256 xStep2 = _mm256_sub_ps(y0, y1);

    __m256 yStep0 = x2x1;
    __m256 yStep1 = x0x2;
    __m256 yStep2 = x1x0;

    weightInit0 = _mm256_sub_ps(weightInit0, _mm256_mul_ps(initMultiplier, xStep0));
    weightInit1 = _mm256_sub_ps(weightInit1, _mm256_mul_ps(initMultiplier, xStep1));
    weightInit2 = _mm256_sub_ps(weightInit2, _mm256_mul_ps(initMultiplier, xStep2));

    xStep0 = _mm256_mul_ps(stepMultiplier, xStep0);
    xStep1 = _mm256_mul_ps(stepMultiplier, xStep1);
    xStep2 = _mm256_mul_ps(stepMultiplier, xStep2);

    uint32* __restrict pixels = ((uint32 * __restrict)(framebuffer)) + (minX + (minY * sMaxWidth));
    float* __restrict pixelDepth = depthBuffer + (minX + (minY * sMaxWidth));

    __m256 weights0 = weightInit0;
    __m256 weights1 = weightInit1;
    __m256 weights2 = weightInit2;

    const int32 simdEnd = minX + (((maxX - minX) >> 3) << 3);
    const int32 stepDelta = sMaxWidth - simdEnd + minX;
    for (int32 h = minY, w = minX; h < maxY;) {
      // OR all weights
      __m256 combinedWeights = _mm256_or_ps(_mm256_or_ps(weights0, weights1), weights2);

      // If all mask bits are set then none of these pixels are inside the triangle.
      if (!_mm256_testc_ps(combinedWeights, signMask)) {
        __m256 depthVec = _mm256_loadu_ps(pixelDepth);

        __m256 zValues = _mm256_rcp_ps(_mm256_fmadd_ps(weights2, z2z0, _mm256_fmadd_ps(weights1, z1z0, z0)));

        __m256 depthMask = _mm256_cmp_ps(zValues, depthVec, _CMP_GT_OQ);

        // Weights are >= 0 when inside, invert when combining masks.
        __m256i finalCombinedMask = _mm256_castps_si256(_mm256_andnot_ps(combinedWeights, depthMask));

        __m256i rValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(_mm256_mul_ps(weights2, r2r0), zValues, _mm256_fmadd_ps(r0, zValues, _mm256_mul_ps(_mm256_mul_ps(weights1, r1r0), zValues))));
        __m256i gValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(_mm256_mul_ps(weights2, g2g0), zValues, _mm256_fmadd_ps(g0, zValues, _mm256_mul_ps(_mm256_mul_ps(weights1, g1g0), zValues))));
        __m256i bValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(_mm256_mul_ps(weights2, b2b0), zValues, _mm256_fmadd_ps(b0, zValues, _mm256_mul_ps(_mm256_mul_ps(weights1, b1b0), zValues))));

        __m256i finalColor = initialColor;
        finalColor = _mm256_or_si256(finalColor, _mm256_slli_epi32(rValues, 16));
        finalColor = _mm256_or_si256(finalColor, _mm256_slli_epi32(gValues, 8));
        finalColor = _mm256_or_si256(finalColor, bValues);

        _mm256_maskstore_epi32((int*)pixels, finalCombinedMask, finalColor);
        _mm256_maskstore_ps(pixelDepth, finalCombinedMask, zValues);
      }

      if (w == simdEnd) {
        w = minX;
        ++h;
        pixels += stepDelta;
        pixelDepth += stepDelta;
        weightInit0 = _mm256_sub_ps(weightInit0, yStep0);
        weightInit1 = _mm256_sub_ps(weightInit1, yStep1);
        weightInit2 = _mm256_sub_ps(weightInit2, yStep2);
        weights0 = weightInit0;
        weights1 = weightInit1;
        weights2 = weightInit2;
      }
      else {
        w += 8;
        pixels += 8;
        pixelDepth += 8;
        weights0 = _mm256_sub_ps(weights0, xStep0);
        weights1 = _mm256_sub_ps(weights1, xStep1);
        weights2 = _mm256_sub_ps(weights2, xStep2);
      }
    }
  }
}

void Unaligned_Shader_UV_SSE(const float* __restrict vertices, const int32* __restrict indices, const int32 end, const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer, const Texture* __restrict texture, size_t mipLevel) {
  const int32 sMaxWidth = (int32)maxWidth;
  // We want the UV values to be scaled by the width/height.
  // Doing that here saves us from having to do that at each pixel.
  // We must still multiply the stride and channels separately because of rounding error.
  size_t texWidth = texture->Width(mipLevel);
  size_t texHeight = texture->Height(mipLevel);
  uint32* __restrict textureData = (uint32 * __restrict)texture->Data(mipLevel);

  __m128 yStride = _mm_set_ps1((float)(texHeight));

  __m128 maxUValue = _mm_set_ps1((float)(texWidth - 1));
  __m128 maxVValue = _mm_set_ps1((float)(texHeight - 1));

  __m128 initMultiplier = _mm_set_ps(3.f, 2.f, 1.f, 0.f);
  __m128 stepMultiplier = _mm_set_ps1(4.f);
  __m128i weightMask = _mm_set1_epi32(0x80000000);

  for (int32 i = 0; i < end; i += 3) {
    const float* __restrict v1 = vertices + indices[i];
    const float* __restrict v2 = vertices + indices[i + 1];
    const float* __restrict v3 = vertices + indices[i + 2];

    __m128 v1All = _mm_loadu_ps(v1);
    __m128 v2All = _mm_loadu_ps(v2);
    __m128 v3All = _mm_loadu_ps(v3);
    __m128 v1Attrs = _mm_loadu_ps(v1 + 4);
    __m128 v2Attrs = _mm_loadu_ps(v2 + 4);
    __m128 v3Attrs = _mm_loadu_ps(v3 + 4);

    __m128 x0 = _mm_shuffle_ps(v1All, v1All, 0b00000000);
    __m128 x1 = _mm_shuffle_ps(v2All, v2All, 0b00000000);
    __m128 x2 = _mm_shuffle_ps(v3All, v3All, 0b00000000);

    __m128 y0 = _mm_shuffle_ps(v1All, v1All, 0b01010101);
    __m128 y1 = _mm_shuffle_ps(v2All, v2All, 0b01010101);
    __m128 y2 = _mm_shuffle_ps(v3All, v3All, 0b01010101);

    __m128 fmins = _mm_min_ps(_mm_min_ps(v1All, v2All), v3All);
    __m128 fmaxs = _mm_max_ps(_mm_max_ps(v1All, v2All), v3All);

    fmins = _mm_round_ps(fmins, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
    fmaxs = _mm_round_ps(fmaxs, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);

    __m128 fminX = _mm_shuffle_ps(fmins, fmins, 0b00000000);
    __m128 fminY = _mm_shuffle_ps(fmins, fmins, 0b01010101);

    __m128i imins = _mm_cvtps_epi32(fmins);
    __m128i imaxs = _mm_cvtps_epi32(fmaxs);

    long long minXY = _mm_cvtsi128_si64(imins);
    long long maxXY = _mm_cvtsi128_si64(imaxs);

    int32 minX = (int32)minXY;
    int32 minY = minXY >> 32;

    int32 maxX = (int32)maxXY;
    int32 maxY = maxXY >> 32;

    __m128 x1x0 = _mm_sub_ps(x1, x0);
    __m128 x2x1 = _mm_sub_ps(x2, x1);
    __m128 x0x2 = _mm_sub_ps(x0, x2);
    __m128 y1y0 = _mm_sub_ps(y1, y0);
    __m128 y2y0 = _mm_sub_ps(y2, y0);

    __m128 invArea = _mm_rcp_ps(_mm_sub_ps(_mm_mul_ps(_mm_sub_ps(x2, x0), y1y0), _mm_mul_ps(y2y0, x1x0)));

    __m128 z0 = _mm_shuffle_ps(v1All, v1All, 0b11111111);
    __m128 invVert0 = _mm_mul_ps(z0, invArea);
    __m128 invVert1 = _mm_mul_ps(_mm_shuffle_ps(v2All, v2All, 0b11111111), invArea);
    __m128 invVert2 = _mm_mul_ps(_mm_shuffle_ps(v3All, v3All, 0b11111111), invArea);

    __m128 z1z0 = _mm_sub_ps(invVert1, invVert0);
    __m128 z2z0 = _mm_sub_ps(invVert2, invVert0);

    __m128 uScaleFactor = _mm_mul_ps(invArea, maxUValue);
    __m128 vScaleFactor = _mm_mul_ps(invArea, maxVValue);

    __m128 u0 = _mm_shuffle_ps(v1Attrs, v1Attrs, 0b00000000);
    __m128 invAttr00 = _mm_mul_ps(u0, uScaleFactor);
    __m128 invAttr01 = _mm_mul_ps(_mm_shuffle_ps(v2Attrs, v2Attrs, 0b00000000), uScaleFactor);
    __m128 invAttr02 = _mm_mul_ps(_mm_shuffle_ps(v3Attrs, v3Attrs, 0b00000000), uScaleFactor);
    u0 = _mm_mul_ps(u0, maxUValue);
    __m128 u1u0 = _mm_sub_ps(invAttr01, invAttr00);
    __m128 u2u0 = _mm_sub_ps(invAttr02, invAttr00);

    __m128 v0 = _mm_shuffle_ps(v1Attrs, v1Attrs, 0b01010101);
    __m128 invAttr10 = _mm_mul_ps(v0, vScaleFactor);
    __m128 invAttr11 = _mm_mul_ps(_mm_shuffle_ps(v2Attrs, v2Attrs, 0b01010101), vScaleFactor);
    __m128 invAttr12 = _mm_mul_ps(_mm_shuffle_ps(v3Attrs, v3Attrs, 0b01010101), vScaleFactor);
    v0 = _mm_mul_ps(v0, maxVValue);
    __m128 v1v0 = _mm_sub_ps(invAttr11, invAttr10);
    __m128 v2v0 = _mm_sub_ps(invAttr12, invAttr10);

    // Calculate the step amount for each horizontal and vertical pixel out of the main loop.
    /*
      float boundingBoxMin[2] = {fminX, fminY};
      __m256 weightInit0 = _mm256_set1_ps(BarycentricArea2D(v2, v3, boundingBoxMin));
      __m256 weightInit1 = _mm256_set1_ps(BarycentricArea2D(v3, v1, boundingBoxMin));
      __m256 weightInit2 = _mm256_set1_ps(BarycentricArea2D(v1, v2, boundingBoxMin));
    */
    __m128 weightInit0 = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(fminX, x1), _mm_sub_ps(y2, y1)), _mm_mul_ps(_mm_sub_ps(fminY, y1), x2x1));
    __m128 weightInit1 = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(fminX, x2), _mm_sub_ps(y0, y2)), _mm_mul_ps(_mm_sub_ps(fminY, y2), x0x2));
    __m128 weightInit2 = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(fminX, x0), y1y0), _mm_mul_ps(_mm_sub_ps(fminY, y0), x1x0));

    __m128 xStep0 = _mm_sub_ps(y1, y2);
    __m128 xStep1 = y2y0;
    __m128 xStep2 = _mm_sub_ps(y0, y1);

    __m128 yStep0 = x2x1;
    __m128 yStep1 = x0x2;
    __m128 yStep2 = x1x0;

    weightInit0 = _mm_sub_ps(weightInit0, _mm_mul_ps(initMultiplier, xStep0));
    weightInit1 = _mm_sub_ps(weightInit1, _mm_mul_ps(initMultiplier, xStep1));
    weightInit2 = _mm_sub_ps(weightInit2, _mm_mul_ps(initMultiplier, xStep2));

    xStep0 = _mm_mul_ps(stepMultiplier, xStep0);
    xStep1 = _mm_mul_ps(stepMultiplier, xStep1);
    xStep2 = _mm_mul_ps(stepMultiplier, xStep2);

    for (int32 h = minY; h < maxY; ++h) {
      __m128 weights0 = weightInit0;
      __m128 weights1 = weightInit1;
      __m128 weights2 = weightInit2;

      uint32* __restrict pixels = ((uint32 * __restrict)(framebuffer)) + (minX + (h * sMaxWidth));
      float* __restrict pixelDepth = depthBuffer + (minX + (h * sMaxWidth));

      for (int32 w = minX; w < maxX; w += 4, pixels += 4, pixelDepth += 4) {
        // Fetch all sign bits and OR them together
        __m128 combinedWeights = _mm_or_ps(_mm_or_ps(weights0, weights1), weights2);

        // If all mask bits are set then none of these pixels are inside the triangle.
        if (!_mm_testc_si128(_mm_castps_si128(combinedWeights), weightMask)) {
          // Our 4-wide alignment doesn't match at the moment. Possibly revisit in the future.
          __m128i pixelVec = _mm_loadu_si128((__m128i * __restrict)pixels);
          __m128 depthVec = _mm_loadu_ps(pixelDepth);

          __m128 weightedVerts0 = z0;
          __m128 weightedVerts1 = _mm_mul_ps(weights1, z1z0);
          __m128 weightedVerts2 = _mm_mul_ps(weights2, z2z0);

          __m128 zValues = _mm_rcp_ps(_mm_add_ps(_mm_add_ps(weightedVerts0, weightedVerts1), weightedVerts2));

          __m128 depthMask = _mm_cmp_ps(zValues, depthVec, _CMP_LT_OQ);

          __m128i finalCombinedMask = _mm_castps_si128(_mm_or_ps(combinedWeights, depthMask));

          __m128 weightedAttr00 = _mm_mul_ps(u0, zValues);
          __m128 weightedAttr01 = _mm_mul_ps(_mm_mul_ps(weights1, u1u0), zValues);
          __m128 weightedAttr02 = _mm_mul_ps(_mm_mul_ps(weights2, u2u0), zValues);

          __m128 weightedAttr10 = _mm_mul_ps(v0, zValues);
          __m128 weightedAttr11 = _mm_mul_ps(_mm_mul_ps(weights1, v1v0), zValues);
          __m128 weightedAttr12 = _mm_mul_ps(_mm_mul_ps(weights2, v2v0), zValues);

          __m128 uValues = _mm_add_ps(_mm_add_ps(weightedAttr00, weightedAttr01), weightedAttr02);
          __m128 vValues = _mm_add_ps(_mm_add_ps(weightedAttr10, weightedAttr11), weightedAttr12);

          // Clamp UV so that we don't index outside of the texture.
          uValues = _mm_min_ps(uValues, maxUValue);
          uValues = _mm_max_ps(uValues, _mm_setzero_ps());
          vValues = _mm_min_ps(vValues, maxVValue);
          vValues = _mm_max_ps(vValues, _mm_setzero_ps());

          // We must round prior to multiplying the stride and channels.
          // If this isn't done, we may jump to a completely different set of pixels because of rounding.
          vValues = _mm_floor_ps(vValues);

          vValues = _mm_add_ps(_mm_mul_ps(vValues, yStride), uValues);

          __m128i colorValues = _mm_cvtps_epi32(vValues);

          __m128i tex3 = _mm_loadu_si32(textureData + _mm_extract_epi32(colorValues, 0b11));
          __m128i tex2 = _mm_loadu_si32(textureData + _mm_extract_epi32(colorValues, 0b10));
          __m128i tex1 = _mm_loadu_si32(textureData + _mm_extract_epi32(colorValues, 0b01));
          __m128i tex0 = _mm_loadu_si32(textureData + _mm_extract_epi32(colorValues, 0b00));

          __m128i loadedColors = _mm_unpacklo_epi64(_mm_unpacklo_epi32(tex0, tex1), _mm_unpacklo_epi32(tex2, tex3));

          __m128i writebackColor = _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(loadedColors), _mm_castsi128_ps(pixelVec), _mm_castsi128_ps(finalCombinedMask)));
          __m128 writebackDepth = _mm_blendv_ps(zValues, depthVec, _mm_castsi128_ps(finalCombinedMask));

          _mm_storeu_si128((__m128i * __restrict)pixels, writebackColor);
          _mm_storeu_ps(pixelDepth, writebackDepth);
        }

        weights0 = _mm_sub_ps(weights0, xStep0);
        weights1 = _mm_sub_ps(weights1, xStep1);
        weights2 = _mm_sub_ps(weights2, xStep2);
      }

      weightInit0 = _mm_sub_ps(weightInit0, yStep0);
      weightInit1 = _mm_sub_ps(weightInit1, yStep1);
      weightInit2 = _mm_sub_ps(weightInit2, yStep2);
    }
  }
}

void Unaligned_Shader_UV_AVX(const float* __restrict vertices, const int32* __restrict indices, const int32 end, const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer, const Texture* __restrict texture, size_t mipLevel) {
  const int32 sMaxWidth = (int32)maxWidth;
  // We want the UV values to be scaled by the width/height.
  // Doing that here saves us from having to do that at each pixel.
  // We must still multiply the stride and channels separately because of rounding error.
  size_t texWidth = texture->Width(mipLevel);
  size_t texHeight = texture->Height(mipLevel);
  uint32* __restrict textureData = (uint32 * __restrict)texture->Data(mipLevel);

  __m256 yStride = _mm256_set1_ps((float)(texHeight));

  __m256 maxUValue = _mm256_set1_ps((float)(texWidth - 1));
  __m256 maxVValue = _mm256_set1_ps((float)(texHeight - 1));

  __m256 initMultiplier = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
  __m256 stepMultiplier = _mm256_set1_ps(8.f);
  __m256 signMask = _mm256_set1_ps(-0.f);

  __m256i yShuffle = _mm256_set1_epi32(1);
  __m256i zShuffle = _mm256_set1_epi32(3);
  __m256i uShuffle = _mm256_set1_epi32(4);
  __m256i vShuffle = _mm256_set1_epi32(5);

  for (int32 i = 0; i < end; i += 3) {
    __m256 v1All = _mm256_loadu_ps(vertices + indices[i]);
    __m256 v2All = _mm256_loadu_ps(vertices + indices[i + 1]);
    __m256 v3All = _mm256_loadu_ps(vertices + indices[i + 2]);

    __m256 x0 = _mm256_broadcastss_ps(_mm256_castps256_ps128(v1All));
    __m256 x1 = _mm256_broadcastss_ps(_mm256_castps256_ps128(v2All));
    __m256 x2 = _mm256_broadcastss_ps(_mm256_castps256_ps128(v3All));

    __m256 y0 = _mm256_permutevar8x32_ps(v1All, yShuffle);
    __m256 y1 = _mm256_permutevar8x32_ps(v2All, yShuffle);
    __m256 y2 = _mm256_permutevar8x32_ps(v3All, yShuffle);

    __m128 fmins = _mm_min_ps(_mm_min_ps(_mm256_castps256_ps128(v1All), _mm256_castps256_ps128(v2All)), _mm256_castps256_ps128(v3All));
    __m128 fmaxs = _mm_max_ps(_mm_max_ps(_mm256_castps256_ps128(v1All), _mm256_castps256_ps128(v2All)), _mm256_castps256_ps128(v3All));

    fmins = _mm_round_ps(fmins, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
    fmaxs = _mm_round_ps(fmaxs, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);

    __m256 fminX = _mm256_broadcastss_ps(fmins);
    __m256 fminY = _mm256_permutevar8x32_ps(_mm256_castps128_ps256(fmins), yShuffle);

    __m128i imins = _mm_cvtps_epi32(fmins);
    __m128i imaxs = _mm_cvtps_epi32(fmaxs);

    long long minXY = _mm_cvtsi128_si64(imins);
    long long maxXY = _mm_cvtsi128_si64(imaxs);

    int32 minX = (int32)minXY;
    int32 minY = minXY >> 32;

    int32 maxX = (int32)maxXY;
    int32 maxY = maxXY >> 32;

    __m256 x1x0 = _mm256_sub_ps(x1, x0);
    __m256 x2x1 = _mm256_sub_ps(x2, x1);
    __m256 x0x2 = _mm256_sub_ps(x0, x2);
    __m256 y1y0 = _mm256_sub_ps(y1, y0);
    __m256 y2y0 = _mm256_sub_ps(y2, y0);

    __m256 invArea = _mm256_rcp_ps(_mm256_fmsub_ps(_mm256_sub_ps(x2, x0), y1y0, _mm256_mul_ps(y2y0, x1x0)));

    __m256 z0 = _mm256_permutevar8x32_ps(v1All, zShuffle);
    __m256 invVert0 = _mm256_mul_ps(z0, invArea);
    __m256 invVert1 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v2All, zShuffle), invArea);
    __m256 invVert2 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v3All, zShuffle), invArea);

    __m256 z1z0 = _mm256_sub_ps(invVert1, invVert0);
    __m256 z2z0 = _mm256_sub_ps(invVert2, invVert0);

    __m256 uScaleFactor = _mm256_mul_ps(invArea, maxUValue);
    __m256 vScaleFactor = _mm256_mul_ps(invArea, maxVValue);

    __m256 u0 = _mm256_permutevar8x32_ps(v1All, uShuffle);
    __m256 invAttr00 = _mm256_mul_ps(u0, uScaleFactor);
    __m256 invAttr01 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v2All, uShuffle), uScaleFactor);
    __m256 invAttr02 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v3All, uShuffle), uScaleFactor);
    u0 = _mm256_mul_ps(u0, maxUValue);
    __m256 u1u0 = _mm256_sub_ps(invAttr01, invAttr00);
    __m256 u2u0 = _mm256_sub_ps(invAttr02, invAttr00);

    __m256 v0 = _mm256_permutevar8x32_ps(v1All, vShuffle);
    __m256 invAttr10 = _mm256_mul_ps(v0, vScaleFactor);
    __m256 invAttr11 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v2All, vShuffle), vScaleFactor);
    __m256 invAttr12 = _mm256_mul_ps(_mm256_permutevar8x32_ps(v3All, vShuffle), vScaleFactor);
    v0 = _mm256_mul_ps(v0, maxVValue);
    __m256 v1v0 = _mm256_sub_ps(invAttr11, invAttr10);
    __m256 v2v0 = _mm256_sub_ps(invAttr12, invAttr10);

    // Calculate the step amount for each horizontal and vertical pixel out of the main loop.
    /*
      float boundingBoxMin[2] = {fminX, fminY};
      __m256 weightInit0 = _mm256_set1_ps(BarycentricArea2D(v2, v3, boundingBoxMin));
      __m256 weightInit1 = _mm256_set1_ps(BarycentricArea2D(v3, v1, boundingBoxMin));
      __m256 weightInit2 = _mm256_set1_ps(BarycentricArea2D(v1, v2, boundingBoxMin));
    */
    __m256 weightInit0 = _mm256_fmsub_ps(_mm256_sub_ps(fminX, x1), _mm256_sub_ps(y2, y1), _mm256_mul_ps(_mm256_sub_ps(fminY, y1), x2x1));
    __m256 weightInit1 = _mm256_fmsub_ps(_mm256_sub_ps(fminX, x2), _mm256_sub_ps(y0, y2), _mm256_mul_ps(_mm256_sub_ps(fminY, y2), x0x2));
    __m256 weightInit2 = _mm256_fmsub_ps(_mm256_sub_ps(fminX, x0), y1y0, _mm256_mul_ps(_mm256_sub_ps(fminY, y0), x1x0));

    __m256 xStep0 = _mm256_sub_ps(y1, y2);
    __m256 xStep1 = y2y0;
    __m256 xStep2 = _mm256_sub_ps(y0, y1);

    __m256 yStep0 = x2x1;
    __m256 yStep1 = x0x2;
    __m256 yStep2 = x1x0;

    weightInit0 = _mm256_sub_ps(weightInit0, _mm256_mul_ps(initMultiplier, xStep0));
    weightInit1 = _mm256_sub_ps(weightInit1, _mm256_mul_ps(initMultiplier, xStep1));
    weightInit2 = _mm256_sub_ps(weightInit2, _mm256_mul_ps(initMultiplier, xStep2));

    xStep0 = _mm256_mul_ps(stepMultiplier, xStep0);
    xStep1 = _mm256_mul_ps(stepMultiplier, xStep1);
    xStep2 = _mm256_mul_ps(stepMultiplier, xStep2);

    uint32* __restrict pixels = ((uint32 * __restrict)(framebuffer)) + (minX + (minY * sMaxWidth));
    float* __restrict pixelDepth = depthBuffer + (minX + (minY * sMaxWidth));

    __m256 weights0 = weightInit0;
    __m256 weights1 = weightInit1;
    __m256 weights2 = weightInit2;

    const int32 simdEnd = minX + (((maxX - minX) >> 3) << 3);
    const int32 stepDelta = sMaxWidth - simdEnd + minX;
    for (int32 h = minY, w = minX; h < maxY;) {
      // OR all weights
      __m256 combinedWeights = _mm256_or_ps(_mm256_or_ps(weights0, weights1), weights2);

      // If all mask bits are set then none of these pixels are inside the triangle.
      if (!_mm256_testc_ps(combinedWeights, signMask)) {
        __m256 depthVec = _mm256_loadu_ps(pixelDepth);

        __m256 zValues = _mm256_rcp_ps(_mm256_fmadd_ps(weights2, z2z0, _mm256_fmadd_ps(weights1, z1z0, z0)));

        __m256 depthMask = _mm256_cmp_ps(zValues, depthVec, _CMP_GT_OQ);

        // Weights are >= 0 when inside, invert when combining masks.
        __m256i finalCombinedMask = _mm256_castps_si256(_mm256_andnot_ps(combinedWeights, depthMask));

        __m256 uValues = _mm256_fmadd_ps(_mm256_mul_ps(weights2, u2u0), zValues, _mm256_fmadd_ps(u0, zValues, _mm256_mul_ps(_mm256_mul_ps(weights1, u1u0), zValues)));
        __m256 vValues = _mm256_fmadd_ps(_mm256_mul_ps(weights2, v2v0), zValues, _mm256_fmadd_ps(v0, zValues, _mm256_mul_ps(_mm256_mul_ps(weights1, v1v0), zValues)));

        // We must round prior to multiplying the stride and channels.
        // If this isn't done, we may jump to a completely different set of pixels because of rounding.
        vValues = _mm256_floor_ps(vValues);

        uValues = _mm256_max_ps(_mm256_min_ps(uValues, maxUValue), _mm256_setzero_ps());
        vValues = _mm256_max_ps(_mm256_min_ps(vValues, maxVValue), _mm256_setzero_ps());

        __m256i colorValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(vValues, yStride, uValues));

        // Note: we're assuming texture data is stored in ARGB format.
        // If it isn't, we need to shuffle and handle alpha here as well.
        // This can be handled outside of the render loop in the texture loading code.

        // NOTE: Gathers are faster in the general case except on some early HW that didn't optimize for it!
        //  If we plan on optimizing for all cases, we will need to take this into account.
        //  This memory read is by far the biggest bottleneck here.
        __m256i loadedColors = _mm256_mask_i32gather_epi32(finalCombinedMask, (const int*)textureData, colorValues, finalCombinedMask, 4);

        _mm256_maskstore_epi32((int*)pixels, finalCombinedMask, loadedColors);
        _mm256_maskstore_ps(pixelDepth, finalCombinedMask, zValues);
      }

      if (w == simdEnd) {
        w = minX;
        ++h;
        pixels += stepDelta;
        pixelDepth += stepDelta;
        weightInit0 = _mm256_sub_ps(weightInit0, yStep0);
        weightInit1 = _mm256_sub_ps(weightInit1, yStep1);
        weightInit2 = _mm256_sub_ps(weightInit2, yStep2);
        weights0 = weightInit0;
        weights1 = weightInit1;
        weights2 = weightInit2;
      }
      else {
        w += 8;
        pixels += 8;
        pixelDepth += 8;
        weights0 = _mm256_sub_ps(weights0, xStep0);
        weights1 = _mm256_sub_ps(weights1, xStep1);
        weights2 = _mm256_sub_ps(weights2, xStep2);
      }
    }
  }
}

}

#endif

#endif
