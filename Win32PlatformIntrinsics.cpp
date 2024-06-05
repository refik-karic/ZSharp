#ifdef PLATFORM_WINDOWS

#include "PlatformIntrinsics.h"

#include "ZBaseTypes.h"
#include "ZAssert.h"
#include "PlatformDefines.h"
#include "ZColor.h"
#include "CommonMath.h"

#include <cstring>

#ifdef HW_PLATFORM_X86

#include <immintrin.h>
#include <intrin.h>

#include "Common.h"

int* CPUIDSection00() {
  static int buffer[4] = {};

  if (buffer[0] == 0) {
    __cpuid(buffer, 0x00);

    // Note: GenuineIntel is not written as expected so we need to swap the last couple DWORDS
    ZSharp::Swap(buffer[3], buffer[2]);
    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
}

int* CPUIDSection01() {
  static int buffer[4] = {};

  if (buffer[2] == 0) {
    __cpuid(buffer, 0x01);

    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
}

int* CPUIDSection07() {
  static int buffer[4] = {};

  if (buffer[2] == 0) {
    __cpuid(buffer, 0x07);

    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
}

int* CPUIDSectionBrand() {
  static int buffer[12] = {};

  if (buffer[0] == 0) {
    __cpuid(buffer, 0x80000002);
    __cpuid(buffer + 4, 0x80000003);
    __cpuid(buffer + 8, 0x80000004);

    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
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

namespace ZSharp {

bool PlatformSupportsSIMDLanes(SIMDLaneWidth width) {
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
      int* bits = CPUIDSection01();
      return bits[2] & (1 << 19);
    }
    break;
    case SIMDLaneWidth::Eight: // AVX2
    {
      int* bits = CPUIDSection07();
      return bits[1] & (1 << 5);
    }
    break;
    case SIMDLaneWidth::Sixteen: // AVX512
    {
      int* bits = CPUIDSection07();
      return bits[1] & (1 << 16);
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
  String vendor((const char*)(CPUIDSection00() + 1), 0, 12);
  return vendor;
}

String PlatformCPUBrand() {
  String brand((const char*)CPUIDSectionBrand(), 0, 48);
  return brand;
}

void Aligned_128Add(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128Sub(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_sub_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128MulByValue(const float* a, const float b, float* dest) {
  _mm_store_ps(dest, _mm_mul_ps(_mm_load_ps(a), _mm_set_ps1(b)));
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

    for (; (j + 8) < rgbBytes; i += 8, j += 8) {
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

    for (; (i + 4) < rgbBytes; i += 4, j += 4) {
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

    for (; (i + 8) < rgbBytes; i += 6, j += 8) {
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

    for (; (i + 4) < rgbBytes; i += 3, j += 4) {
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

    for (size_t h = 0; h < height; ++h) {
      uint32* currentPixels = image + (h * width);

      size_t w = 0;
      for (; (w + 8) < width; w += 8, currentPixels += 8) {
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

    for (size_t h = 0; h < height; ++h) {
      uint32* currentPixels = image + (h * width);

      size_t w = 0;
      for (; (w + 4) < width; w += 4, currentPixels += 4) {
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

void Unaligned_GenerateMipLevel(uint8* nextMip, size_t nextWidth, size_t nextHeight, uint8* lastMip, size_t lastWidth, size_t lastHeight) {
  (void)lastHeight;
  size_t nextMipStride = nextWidth * 4;
  size_t lastMipStride = lastWidth * 4;
  
  __m128i shuffleLeft = _mm_set_epi8(
    0x80U, 0x80U, 0x80U, 3,
    0x80U, 0x80U, 0x80U, 2,
    0x80U, 0x80U, 0x80U, 1,
    0x80U, 0x80U, 0x80U, 0
  );

  __m128i shuffleRight = _mm_set_epi8(
    0x80U, 0x80U, 0x80U, 7,
    0x80U, 0x80U, 0x80U, 6,
    0x80U, 0x80U, 0x80U, 5,
    0x80U, 0x80U, 0x80U, 4
  );

  __m128i shuffleColor = _mm_set_epi8(
    0x80U, 0x80U, 0x80U, 0x80U,
    0x80U, 0x80U, 0x80U, 0x80U,
    0x80U, 0x80U, 0x80U, 0x80U,
    12, 8, 4, 0
  );

  for (size_t y = 0; y < nextHeight; ++y) {
    for (size_t x = 0; x < nextWidth; ++x) {
      const size_t xStride = x * 4;

      uint8* topLeft = lastMip + (y * 2 * lastMipStride) + (xStride * 2);
      uint8* bottomLeft = topLeft + lastMipStride;

      __m128i topData = _mm_loadu_si64(topLeft);
      __m128i bottomData = _mm_loadu_si64(bottomLeft);

      __m128i topLeftData = _mm_shuffle_epi8(topData, shuffleLeft);
      __m128i topRightData = _mm_shuffle_epi8(topData, shuffleRight);
      __m128i bottomLeftData = _mm_shuffle_epi8(bottomData, shuffleLeft);
      __m128i bottomRightData = _mm_shuffle_epi8(bottomData, shuffleRight);

      __m128i rgba = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(topLeftData, topRightData), bottomLeftData), bottomRightData);
      rgba = _mm_srli_epi32(rgba, 2);
      rgba = _mm_shuffle_epi8(rgba, shuffleColor);

      _mm_storeu_si32(nextMip + ((y * nextMipStride) + xStride), rgba);
    }
  }
}

void Aligned_Mat4x4Transform(const float matrix[4][4], float* data, int32 stride, int32 length) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    /*
      We preload 4 registers with all of the matrix X, Y, Z, W values.
      We also broadcast each vector component (X, Y, Z, W) to 4 registers.

      This saves us some arithmetic. We only have to perform 4 multiplies and 4 adds per vertex in the end.
    */

    __m128 matrixX = _mm_set_ps(matrix[3][0], matrix[2][0], matrix[1][0], matrix[0][0]);
    __m128 matrixY = _mm_set_ps(matrix[3][1], matrix[2][1], matrix[1][1], matrix[0][1]);
    __m128 matrixZ = _mm_set_ps(matrix[3][2], matrix[2][2], matrix[1][2], matrix[0][2]);
    __m128 matrixW = _mm_set_ps(matrix[3][3], matrix[2][3], matrix[1][3], matrix[0][3]);

    for (size_t i = 0; i < length; i += stride) {
      float* vecData = data + i;

      __m128 vecX = _mm_set_ps1(vecData[0]);
      __m128 vecY = _mm_set_ps1(vecData[1]);
      __m128 vecZ = _mm_set_ps1(vecData[2]);
      __m128 vecW = _mm_set_ps1(vecData[3]);

      vecX = _mm_mul_ps(matrixX, vecX);
      vecY = _mm_mul_ps(matrixY, vecY);
      vecZ = _mm_mul_ps(matrixZ, vecZ);
      vecW = _mm_mul_ps(matrixW, vecW);

      __m128 result = _mm_add_ps(vecX, vecY);
      result = _mm_add_ps(result, vecZ);
      result = _mm_add_ps(result, vecW);

      _mm_storeu_ps(vecData, result);
    }
  }
  else {
    for (int32 i = 0; i < length; i += stride) {
      float* vec = (data + i);

      float xyzw[4];
      xyzw[0] = (matrix[0][0] * vec[0]) + (matrix[0][1] * vec[1]) + (matrix[0][2] * vec[2]) + (matrix[0][3] * vec[3]);
      xyzw[1] = (matrix[1][0] * vec[0]) + (matrix[1][1] * vec[1]) + (matrix[1][2] * vec[2]) + (matrix[1][3] * vec[3]);
      xyzw[2] = (matrix[2][0] * vec[0]) + (matrix[2][1] * vec[1]) + (matrix[2][2] * vec[2]) + (matrix[2][3] * vec[3]);
      xyzw[3] = (matrix[3][0] * vec[0]) + (matrix[3][1] * vec[1]) + (matrix[3][2] * vec[2]) + (matrix[3][3] * vec[3]);

      memcpy(data + i, xyzw, sizeof(xyzw));
    }
  }
}

void Aligned_DepthBufferVisualize(float* buffer, size_t width, size_t height) {
  float* pixel = buffer;

  ZColor black(ZColors::BLACK);
  ZColor white(ZColors::WHITE);

  // TODO: Find a better way to scale the depth values.
  //  This is just a good guess.
  float invDenominatorScalar = 1.f / -6.f;

  size_t currentIndex = 0;

  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    __m256 subFactor = _mm256_set1_ps(6.f);
    __m256 invDenominator = _mm256_set1_ps(invDenominatorScalar);

    __m256 rbVec = _mm256_set1_ps((float)black.R());
    __m256 gbVec = _mm256_set1_ps((float)black.G());
    __m256 bbVec = _mm256_set1_ps((float)black.B());

    __m256 rwVec = _mm256_set1_ps((float)white.R());
    __m256 gwVec = _mm256_set1_ps((float)white.G());
    __m256 bwVec = _mm256_set1_ps((float)white.B());

    __m256i initialColor = _mm256_set1_epi32(0xFF000000);

    for (size_t h = 0; h < height; ++h) {
      for (size_t w = 0; w + 8 < width; w += 8, currentIndex += 8, pixel += 8) {
        __m256 numerator = _mm256_load_ps(pixel);
        numerator = _mm256_sub_ps(numerator, subFactor);
        __m256 parametricT = _mm256_mul_ps(numerator, invDenominator);

        __m256 lerpedR = Lerp256(rbVec, rwVec, parametricT);
        __m256 lerpedG = Lerp256(gbVec, gwVec, parametricT);
        __m256 lerpedB = Lerp256(bbVec, bwVec, parametricT);

        __m256i convR = _mm256_cvtps_epi32(lerpedR);
        __m256i convG = _mm256_cvtps_epi32(lerpedG);
        __m256i convB = _mm256_cvtps_epi32(lerpedB);

        convR = _mm256_slli_epi32(convR, 16);
        convG = _mm256_slli_epi32(convG, 8);

        __m256i finalColor = initialColor;
        finalColor = _mm256_or_si256(finalColor, convR);
        finalColor = _mm256_or_si256(finalColor, convG);
        finalColor = _mm256_or_si256(finalColor, convB);

        _mm256_storeu_si256((__m256i*)pixel, finalColor);
      }
    }
  }
  else {
    __m128 subFactor = _mm_set_ps1(6.f);
    __m128 invDenominator = _mm_set_ps1(invDenominatorScalar);

    __m128 rbVec = _mm_set_ps1((float)black.R());
    __m128 gbVec = _mm_set_ps1((float)black.G());
    __m128 bbVec = _mm_set_ps1((float)black.B());

    __m128 rwVec = _mm_set_ps1((float)white.R());
    __m128 gwVec = _mm_set_ps1((float)white.G());
    __m128 bwVec = _mm_set_ps1((float)white.B());

    __m128i initialColor = _mm_set1_epi32(0xFF000000);

    for (size_t h = 0; h < height; ++h) {
      for (size_t w = 0; w + 4 < width; w += 4, currentIndex += 4, pixel += 4) {
        __m128 numerator = _mm_load_ps(pixel);
        numerator = _mm_sub_ps(numerator, subFactor);
        __m128 parametricT = _mm_mul_ps(numerator, invDenominator);

        __m128 lerpedR = Lerp128(rbVec, rwVec, parametricT);
        __m128 lerpedG = Lerp128(gbVec, gwVec, parametricT);
        __m128 lerpedB = Lerp128(bbVec, bwVec, parametricT);

        __m128i convR = _mm_cvtps_epi32(lerpedR);
        __m128i convG = _mm_cvtps_epi32(lerpedG);
        __m128i convB = _mm_cvtps_epi32(lerpedB);

        convR = _mm_slli_epi32(convR, 16);
        convG = _mm_slli_epi32(convG, 8);

        __m128i finalColor = initialColor;
        finalColor = _mm_or_si128(finalColor, convR);
        finalColor = _mm_or_si128(finalColor, convG);
        finalColor = _mm_or_si128(finalColor, convB);

        _mm_storeu_si128((__m128i*)pixel, finalColor);
      }
    }
  }

  size_t endBuffer = width * height;

  for (size_t w = currentIndex; w < endBuffer; ++w) {
    float numerator = *pixel - 6.f;
    float t = numerator * invDenominatorScalar;

    (*((uint32*)pixel)) = ZColor::LerpColors(black, white, t);
    ++pixel;
  }
}

void Aligned_Vec4Homogenize(float* data, int32 stride, int32 length) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    for (int32 i = 0; i < length; i += stride) {
      float* nextVec = data + i;
      __m128 vec = _mm_loadu_ps(nextVec);
      __m128 perspectiveTerm = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), 0b11111111));
      __m128 result = _mm_mul_ps(vec, _mm_rcp_ps(perspectiveTerm));
      result = _mm_blend_ps(result, perspectiveTerm, 0b1000);
      _mm_storeu_ps(nextVec, result);
    }
  }
  else {
    for (int32 i = 0; i < length; i += stride) {
      float* vec = data + i;
      const float invDivisor = 1.f / vec[3];
      vec[0] *= invDivisor;
      vec[1] *= invDivisor;
      vec[2] *= invDivisor;
    }
  }
}

void Unaligned_BlendBuffers(uint32* devBuffer, uint32* frameBuffer, size_t width, size_t height, float opacity) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    __m256i devOpacity = _mm256_set1_epi16((short)(255.f * opacity));
    __m256i bufferOpacity = _mm256_set1_epi16((short)(255.f * (1.f - opacity)));

    // Need this to clear high 8 bits of the 16 unpacked value
    __m256i zero = _mm256_setzero_si256();

    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; x += 8) {
        size_t index = (y * width) + x;

        // Load 8 pixels at a time
        // Split the blend function into two halves because SSE doesn't have artihmetic insns for 8bit values
        // The best we can do is 16bit arithmetic
        __m256i devColor = _mm256_loadu_si256((__m256i*)(devBuffer + index));
        __m256i bufferColor = _mm256_loadu_si256((__m256i*)(frameBuffer + index));

        __m256i devLo16 = _mm256_unpacklo_epi8(devColor, zero); // 4x 32bit BGRA, [0,1]
        __m256i devHi16 = _mm256_unpackhi_epi8(devColor, zero); // 4x 32bit BGRA, [2,3]

        __m256i bufLo16 = _mm256_unpacklo_epi8(bufferColor, zero);
        __m256i bufHi16 = _mm256_unpackhi_epi8(bufferColor, zero);

        devLo16 = _mm256_mullo_epi16(devLo16, devOpacity);
        devHi16 = _mm256_mullo_epi16(devHi16, devOpacity);

        bufLo16 = _mm256_mullo_epi16(bufLo16, bufferOpacity);
        bufHi16 = _mm256_mullo_epi16(bufHi16, bufferOpacity);

        // Divide by 256
        devLo16 = _mm256_srli_epi16(devLo16, 8);
        devHi16 = _mm256_srli_epi16(devHi16, 8);

        bufLo16 = _mm256_srli_epi16(bufLo16, 8);
        bufHi16 = _mm256_srli_epi16(bufHi16, 8);

        __m256i loResult = _mm256_add_epi16(devLo16, bufLo16);
        __m256i hiResult = _mm256_add_epi16(devHi16, bufHi16);

        // Pack the 16bit results into 8bit values and store it back to memory
        _mm256_storeu_si256((__m256i*)(frameBuffer + index), _mm256_packus_epi16(loResult, hiResult));
      }
    }
  }
  else {
    __m128i devOpacity = _mm_set1_epi16((short)(255.f * opacity));
    __m128i bufferOpacity = _mm_set1_epi16((short)(255.f * (1.f - opacity)));

    // Need this to clear high 8 bits of the 16 unpacked value
    __m128i zero = _mm_setzero_si128();

    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; x += 4) {
        size_t index = (y * width) + x;

        // Load 4 pixels at a time
        // Split the blend function into two halves because SSE doesn't have artihmetic insns for 8bit values
        // The best we can do is 16bit arithmetic
        __m128i devColor = _mm_loadu_si128((__m128i*)(devBuffer + index));
        __m128i bufferColor = _mm_loadu_si128((__m128i*)(frameBuffer + index));

        __m128i devLo16 = _mm_unpacklo_epi8(devColor, zero); // 2x 32bit BGRA, [0,1]
        __m128i devHi16 = _mm_unpackhi_epi8(devColor, zero); // 2x 32bit BGRA, [2,3]

        __m128i bufLo16 = _mm_unpacklo_epi8(bufferColor, zero);
        __m128i bufHi16 = _mm_unpackhi_epi8(bufferColor, zero);

        devLo16 = _mm_mullo_epi16(devLo16, devOpacity);
        devHi16 = _mm_mullo_epi16(devHi16, devOpacity);

        bufLo16 = _mm_mullo_epi16(bufLo16, bufferOpacity);
        bufHi16 = _mm_mullo_epi16(bufHi16, bufferOpacity);

        // Divide by 256
        devLo16 = _mm_srli_epi16(devLo16, 8);
        devHi16 = _mm_srli_epi16(devHi16, 8);

        bufLo16 = _mm_srli_epi16(bufLo16, 8);
        bufHi16 = _mm_srli_epi16(bufHi16, 8);

        __m128i loResult = _mm_add_epi16(devLo16, bufLo16);
        __m128i hiResult = _mm_add_epi16(devHi16, bufHi16);

        // Pack the 16bit results into 8bit values and store it back to memory
        _mm_storeu_si128((__m128i*)(frameBuffer + index), _mm_packus_epi16(loResult, hiResult));
      }
    }
  }
}

void Aligned_BackfaceCull(IndexBuffer& indexBuffer, const VertexBuffer& vertexBuffer, const float viewer[3]) {
  __m128 view = _mm_loadu_ps(viewer);

  int32* indexData = indexBuffer.GetInputData();
  const float* vertexData = vertexBuffer[0];

  __m128i dotSign = _mm_set_epi32(0, 0, 0, 0x80000000);

  for (int32 i = indexBuffer.GetIndexSize(); i >= 3; i -= 3) {
    int32 i1 = indexData[i - 3];
    int32 i2 = indexData[i - 2];
    int32 i3 = indexData[i - 1];
    __m128 v1 = _mm_loadu_ps(vertexData + i1);
    __m128 v2 = _mm_loadu_ps(vertexData + i2);
    __m128 v3 = _mm_loadu_ps(vertexData + i3);

    __m128 p1p0 = _mm_sub_ps(v2, v1);
    __m128 p2p0 = _mm_sub_ps(v3, v1);

    __m128 p1p0Shuffled0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p1p0), 0b11001001));
    __m128 p1p0Shuffled1 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p1p0), 0b11010010));

    __m128 p2p0Shuffled0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p2p0), 0b11010010));
    __m128 p2p0Shuffled1 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p2p0), 0b11001001));

    __m128 normal = _mm_sub_ps(_mm_mul_ps(p1p0Shuffled0, p2p0Shuffled0), _mm_mul_ps(p1p0Shuffled1, p2p0Shuffled1));

    __m128 dotIntermediate = _mm_mul_ps(_mm_sub_ps(v1, view), normal);

    dotIntermediate = _mm_hadd_ps(_mm_hadd_ps(dotIntermediate, dotIntermediate), _mm_setzero_ps());

    if (_mm_testz_si128(_mm_castps_si128(dotIntermediate), dotSign)) {
      indexBuffer.RemoveTriangle(i - 3);
    }
  }
}

void Aligned_WindowTransform(float* data, int32 stride, int32 length, const float windowTransform0[3], const float windowTransform1[3], const float width, const float height) {
  __m128 window0 = _mm_set_ps(0.f, windowTransform0[2], windowTransform0[1], windowTransform0[0]);
  __m128 window1 = _mm_set_ps(0.f, windowTransform1[2], windowTransform1[1], windowTransform1[0]);

  __m128 maxXY = _mm_set_ps(0.f, 0.f, height, width);

  for (int32 i = 0; i < length; ++i) {
    float* vertexData = data + (i * stride);

    __m128 vec = _mm_loadu_ps(vertexData);
    __m128 perspectiveZ = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), 0b11111111));
    __m128 invPerspectiveZ = _mm_rcp_ps(perspectiveZ);
    __m128 invDivisor = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), 0b10101010)));

    // [0] = _
    // [1] = _
    // [2] = perspectiveZ
    // [3] = invPerspectiveZ
    perspectiveZ = _mm_blend_ps(perspectiveZ, invPerspectiveZ, 0b1000);

    // Homogenize with Z
    __m128 result = _mm_mul_ps(vec, invDivisor);

    // Apply Window transform.
    __m128 dotX = _mm_mul_ps(result, window0);
    __m128 dotY = _mm_mul_ps(result, window1);

    result = _mm_hadd_ps(_mm_hadd_ps(dotX, dotY), _mm_setzero_ps());
    result = _mm_min_ps(_mm_max_ps(result, _mm_setzero_ps()), maxXY);

    // [0] = dotX
    // [1] = dotY
    // [2] = perspectiveZ
    // [3] = invPerspectiveZ
    result = _mm_blend_ps(result, perspectiveZ, 0b1100);

    _mm_storeu_ps(vertexData, result);

    /*
      NOTE: In most cases this is actually faster than doing a load and masked store.
        Main theory being, we don't typically need to write the entire register back to memory.
    */
    for (int32 j = 4; j < stride; ++j) {
      _mm_store_ss(vertexData + j, _mm_mul_ss(_mm_load_ss(vertexData + j), invPerspectiveZ));
    }
  }
}

void Aligned_TransformDirectScreenSpace(float* data, int32 stride, int32 length, const float matrix[4][4], const float windowTransform0[3], const float windowTransform1[3], const float width, const float height) {
  __m128 matrixX = _mm_set_ps(matrix[3][0], matrix[2][0], matrix[1][0], matrix[0][0]);
  __m128 matrixY = _mm_set_ps(matrix[3][1], matrix[2][1], matrix[1][1], matrix[0][1]);
  __m128 matrixZ = _mm_set_ps(matrix[3][2], matrix[2][2], matrix[1][2], matrix[0][2]);
  __m128 matrixW = _mm_set_ps(matrix[3][3], matrix[2][3], matrix[1][3], matrix[0][3]);

  __m128 window0 = _mm_set_ps(0.f, windowTransform0[2], windowTransform0[1], windowTransform0[0]);
  __m128 window1 = _mm_set_ps(0.f, windowTransform1[2], windowTransform1[1], windowTransform1[0]);

  __m128 maxXY = _mm_set_ps(0.f, 0.f, height, width);

  __m128i xShuffle = _mm_set_epi8(
    3, 2, 1, 0,
    3, 2, 1, 0,
    3, 2, 1, 0,
    3, 2, 1, 0
  );

  __m128i yShuffle = _mm_set_epi8(
    7, 6, 5, 4,
    7, 6, 5, 4,
    7, 6, 5, 4,
    7, 6, 5, 4
  );

  __m128i zShuffle = _mm_set_epi8(
    11, 10, 9, 8,
    11, 10, 9, 8,
    11, 10, 9, 8,
    11, 10, 9, 8
  );

  __m128i wShuffle = _mm_set_epi8(
    15, 14, 13, 12,
    15, 14, 13, 12,
    15, 14, 13, 12,
    15, 14, 13, 12
  );

  for (size_t i = 0; i < length; i += stride) {
    float* vecData = data + i;

    // Perspective projection
    __m128 xyzw = _mm_loadu_ps(vecData);

    __m128 vecX = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(xyzw), xShuffle));
    __m128 vecY = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(xyzw), yShuffle));
    __m128 vecZ = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(xyzw), zShuffle));
    __m128 vecW = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(xyzw), wShuffle));

    vecX = _mm_mul_ps(matrixX, vecX);
    vecY = _mm_mul_ps(matrixY, vecY);
    vecZ = _mm_mul_ps(matrixZ, vecZ);
    vecW = _mm_mul_ps(matrixW, vecW);

    __m128 vec = _mm_add_ps(vecX, vecY);
    vec = _mm_add_ps(vec, vecZ);
    vec = _mm_add_ps(vec, vecW);

    // Homogenize
    __m128 perspectiveTerm = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), 0b11111111));
    __m128 invPerspectiveZ = _mm_rcp_ps(perspectiveTerm);
    __m128 homogenized = _mm_mul_ps(vec, invPerspectiveZ);

    // Window transform
    __m128 invDivisor = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(homogenized), 0b10101010)));

    // [0] = _
    // [1] = _
    // [2] = perspectiveZ
    // [3] = invPerspectiveZ
    perspectiveTerm = _mm_blend_ps(perspectiveTerm, invPerspectiveZ, 0b1000);

    // Homogenize with Z
    __m128 result = _mm_mul_ps(homogenized, invDivisor);

    // Apply Window transform.
    __m128 dotX = _mm_mul_ps(result, window0);
    __m128 dotY = _mm_mul_ps(result, window1);

    result = _mm_hadd_ps(_mm_hadd_ps(dotX, dotY), _mm_setzero_ps());
    result = _mm_min_ps(_mm_max_ps(result, _mm_setzero_ps()), maxXY);

    // [0] = dotX
    // [1] = dotY
    // [2] = perspectiveZ
    // [3] = invPerspectiveZ
    result = _mm_blend_ps(result, perspectiveTerm, 0b1100);

    _mm_storeu_ps(vecData, result);

    /*
      NOTE: In most cases this is actually faster than doing a load and masked store.
        Main theory being, we don't typically need to write the entire register back to memory.
    */
    for (int32 j = 4; j < stride; ++j) {
      _mm_store_ss(vecData + j, _mm_mul_ss(_mm_load_ss(vecData + j), invPerspectiveZ));
    }
  }
}

void Unaligned_AABB(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]) {
  /*
  Note that we don't do any kind of CPUID check here.
  minps/maxps are available since the first version of SSE.
  */
  
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

void Unaligned_FlatShadeRGB(const float* vertices, const int32* indices, const int32 end, const float maxWidth, uint8* framebuffer, float* depthBuffer) {
  const int32 sMaxWidth = (int32)maxWidth;
  
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    for (int32 i = 0; i < end; i += 3) {
      const float* v1 = vertices + indices[i];
      const float* v2 = vertices + indices[i + 1];
      const float* v3 = vertices + indices[i + 2];

      __m256 x0 = _mm256_broadcastss_ps(_mm_load_ps1(v1));
      __m256 x1 = _mm256_broadcastss_ps(_mm_load_ps1(v2));
      __m256 x2 = _mm256_broadcastss_ps(_mm_load_ps1(v3));

      __m256 y0 = _mm256_broadcastss_ps(_mm_load_ps1(v1 + 1));
      __m256 y1 = _mm256_broadcastss_ps(_mm_load_ps1(v2 + 1));
      __m256 y2 = _mm256_broadcastss_ps(_mm_load_ps1(v3 + 1));

      __m256 fminX = _mm256_min_ps(_mm256_min_ps(x0, x1), x2);
      __m256 fminY = _mm256_min_ps(_mm256_min_ps(y0, y1), y2);
      __m256 fmaxX = _mm256_max_ps(_mm256_max_ps(x0, x1), x2);
      __m256 fmaxY = _mm256_max_ps(_mm256_max_ps(y0, y1), y2);

#ifdef __clang__
      fminX = _mm256_round_ps(fminX, _MM_FROUND_FLOOR);
      fminY = _mm256_round_ps(fminY, _MM_FROUND_FLOOR);

      fmaxX = _mm256_round_ps(fmaxX, _MM_FROUND_CEIL);
      fmaxY = _mm256_round_ps(fmaxY, _MM_FROUND_CEIL);
#else
      fminX = _mm256_round_ps(fminX, _MM_ROUND_MODE_DOWN);
      fminY = _mm256_round_ps(fminY, _MM_ROUND_MODE_DOWN);

      fmaxX = _mm256_round_ps(fmaxX, _MM_ROUND_MODE_UP);
      fmaxY = _mm256_round_ps(fmaxY, _MM_ROUND_MODE_UP);
#endif

      int32 minX = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fminX));
      int32 maxX = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fmaxX));
      int32 minY = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fminY));
      int32 maxY = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fmaxY));

      __m256 x1x0 = _mm256_sub_ps(x1, x0);
      __m256 x2x1 = _mm256_sub_ps(x2, x1);
      __m256 x0x2 = _mm256_sub_ps(x0, x2);
      __m256 y1y0 = _mm256_sub_ps(y1, y0);
      __m256 y2y0 = _mm256_sub_ps(y2, y0);

      __m256 invArea = _mm256_rcp_ps(_mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(x2, x0), y1y0), _mm256_mul_ps(y2y0, x1x0)));

      __m256 z0 = _mm256_set1_ps(v1[3]);
      __m256 invVert0 = _mm256_mul_ps(z0, invArea);
      __m256 invVert1 = _mm256_mul_ps(_mm256_set1_ps(v2[3]), invArea);
      __m256 invVert2 = _mm256_mul_ps(_mm256_set1_ps(v3[3]), invArea);

      __m256 z1z0 = _mm256_sub_ps(invVert1, invVert0);
      __m256 z2z0 = _mm256_sub_ps(invVert2, invVert0);

      // We want the RGB values to be scaled by 255 in the end.
      // Doing that here saves us from having to apply the scale at each pixel.
      __m256 rgbScale = _mm256_set1_ps(255.f);
      __m256 scaleFactor = _mm256_mul_ps(invArea, rgbScale);

      __m256 r0 = _mm256_set1_ps(v1[4]);
      __m256 invAttr00 = _mm256_mul_ps(r0, scaleFactor);
      __m256 invAttr01 = _mm256_mul_ps(_mm256_set1_ps(v2[4]), scaleFactor);
      __m256 invAttr02 = _mm256_mul_ps(_mm256_set1_ps(v3[4]), scaleFactor);

      r0 = _mm256_mul_ps(r0, rgbScale);
      __m256 r1r0 = _mm256_sub_ps(invAttr01, invAttr00);
      __m256 r2r0 = _mm256_sub_ps(invAttr02, invAttr00);

      __m256 g0 = _mm256_set1_ps(v1[5]);
      __m256 invAttr10 = _mm256_mul_ps(g0, scaleFactor);
      __m256 invAttr11 = _mm256_mul_ps(_mm256_set1_ps(v2[5]), scaleFactor);
      __m256 invAttr12 = _mm256_mul_ps(_mm256_set1_ps(v3[5]), scaleFactor);

      g0 = _mm256_mul_ps(g0, rgbScale);
      __m256 g1g0 = _mm256_sub_ps(invAttr11, invAttr10);
      __m256 g2g0 = _mm256_sub_ps(invAttr12, invAttr10);

      __m256 b0 = _mm256_set1_ps(v1[6]);
      __m256 invAttr20 = _mm256_mul_ps(b0, scaleFactor);
      __m256 invAttr21 = _mm256_mul_ps(_mm256_set1_ps(v2[6]), scaleFactor);
      __m256 invAttr22 = _mm256_mul_ps(_mm256_set1_ps(v3[6]), scaleFactor);

      b0 = _mm256_mul_ps(b0, rgbScale);
      __m256 b1b0 = _mm256_sub_ps(invAttr21, invAttr20);
      __m256 b2b0 = _mm256_sub_ps(invAttr22, invAttr20);

      // Calculate the step amount for each horizontal and vertical pixel out of the main loop.
      __m256 weightInit0 = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(fminX, x1), _mm256_sub_ps(y2, y1)), _mm256_mul_ps(_mm256_sub_ps(fminY, y1), x2x1));
      __m256 weightInit1 = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(fminX, x2), _mm256_sub_ps(y0, y2)), _mm256_mul_ps(_mm256_sub_ps(fminY, y2), x0x2));
      __m256 weightInit2 = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(fminX, x0), y1y0), _mm256_mul_ps(_mm256_sub_ps(fminY, y0), x1x0));

      __m256 xStep0 = _mm256_sub_ps(y1, y2);
      __m256 xStep1 = y2y0;
      __m256 xStep2 = _mm256_sub_ps(y0, y1);

      __m256 yStep0 = x2x1;
      __m256 yStep1 = x0x2;
      __m256 yStep2 = x1x0;

      __m256 initMultiplier = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
      __m256 stepMultiplier = _mm256_set1_ps(8.f);

      weightInit0 = _mm256_sub_ps(weightInit0, _mm256_mul_ps(initMultiplier, xStep0));
      weightInit1 = _mm256_sub_ps(weightInit1, _mm256_mul_ps(initMultiplier, xStep1));
      weightInit2 = _mm256_sub_ps(weightInit2, _mm256_mul_ps(initMultiplier, xStep2));

      xStep0 = _mm256_mul_ps(stepMultiplier, xStep0);
      xStep1 = _mm256_mul_ps(stepMultiplier, xStep1);
      xStep2 = _mm256_mul_ps(stepMultiplier, xStep2);

      __m256i initialColor = _mm256_set1_epi32(0xFF000000);
      __m256i weightMask = _mm256_set1_epi32(0x80000000);

      for (int32 h = minY; h < maxY; ++h) {
        __m256 weights0 = weightInit0;
        __m256 weights1 = weightInit1;
        __m256 weights2 = weightInit2;

        uint32* pixels = ((uint32*)(framebuffer)) + (minX + (h * sMaxWidth));
        float* pixelDepth = depthBuffer + (minX + (h * sMaxWidth));

        for (int32 w = minX; w < maxX; w += 8, pixels += 8, pixelDepth += 8) {
          // OR all weights and fetch the sign bits
          __m256 combinedWeights = _mm256_or_ps(_mm256_or_ps(weights0, weights1), weights2);

          // If all mask bits are set then none of these pixels are inside the triangle.
          if (!_mm256_testc_si256(_mm256_castps_si256(combinedWeights), weightMask)) {
            __m256 depthVec = _mm256_maskload_ps(pixelDepth, Not256(_mm256_castps_si256(combinedWeights)));

            __m256 zValues = _mm256_fmadd_ps(weights2, z2z0, _mm256_fmadd_ps(weights1, z1z0, z0));

            __m256 invZValues = _mm256_rcp_ps(zValues);

            __m256 depthMask = _mm256_cmp_ps(zValues, depthVec, _CMP_GT_OQ);

            __m256i finalCombinedMask = Not256(_mm256_castps_si256(_mm256_or_ps(combinedWeights, depthMask)));

            __m256i rValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(_mm256_mul_ps(weights2, r2r0), invZValues, _mm256_fmadd_ps(r0, invZValues, _mm256_mul_ps(_mm256_mul_ps(weights1, r1r0), invZValues))));
            __m256i gValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(_mm256_mul_ps(weights2, g2g0), invZValues, _mm256_fmadd_ps(g0, invZValues, _mm256_mul_ps(_mm256_mul_ps(weights1, g1g0), invZValues))));
            __m256i bValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(_mm256_mul_ps(weights2, b2b0), invZValues, _mm256_fmadd_ps(b0, invZValues, _mm256_mul_ps(_mm256_mul_ps(weights1, b1b0), invZValues))));

            rValues = _mm256_slli_epi32(rValues, 16);
            gValues = _mm256_slli_epi32(gValues, 8);

            __m256i finalColor = initialColor;
            finalColor = _mm256_or_si256(finalColor, rValues);
            finalColor = _mm256_or_si256(finalColor, gValues);
            finalColor = _mm256_or_si256(finalColor, bValues);

            _mm256_maskstore_epi32((int*)pixels, finalCombinedMask, finalColor);
            _mm256_maskstore_ps(pixelDepth, finalCombinedMask, zValues);
          }

          weights0 = _mm256_sub_ps(weights0, xStep0);
          weights1 = _mm256_sub_ps(weights1, xStep1);
          weights2 = _mm256_sub_ps(weights2, xStep2);
        }

        weightInit0 = _mm256_sub_ps(weightInit0, yStep0);
        weightInit1 = _mm256_sub_ps(weightInit1, yStep1);
        weightInit2 = _mm256_sub_ps(weightInit2, yStep2);
      }
      
    }
  }
  else {
    for (int32 i = 0; i < end; i += 3) {
      const float* v1 = vertices + indices[i];
      const float* v2 = vertices + indices[i + 1];
      const float* v3 = vertices + indices[i + 2];

      __m128 x0 = _mm_load_ps1(v1);
      __m128 x1 = _mm_load_ps1(v2);
      __m128 x2 = _mm_load_ps1(v3);

      __m128 y0 = _mm_load_ps1(v1 + 1);
      __m128 y1 = _mm_load_ps1(v2 + 1);
      __m128 y2 = _mm_load_ps1(v3 + 1);

      __m128 fminX = _mm_min_ps(_mm_min_ps(x0, x1), x2);
      __m128 fminY = _mm_min_ps(_mm_min_ps(y0, y1), y2);
      __m128 fmaxX = _mm_max_ps(_mm_max_ps(x0, x1), x2);
      __m128 fmaxY = _mm_max_ps(_mm_max_ps(y0, y1), y2);

      fminX = _mm_round_ps(fminX, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
      fminY = _mm_round_ps(fminY, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);

      fmaxX = _mm_round_ps(fmaxX, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);
      fmaxY = _mm_round_ps(fmaxY, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);

      int32 minX = _mm_cvtsi128_si32(_mm_cvtps_epi32(fminX));
      int32 maxX = _mm_cvtsi128_si32(_mm_cvtps_epi32(fmaxX));
      int32 minY = _mm_cvtsi128_si32(_mm_cvtps_epi32(fminY));
      int32 maxY = _mm_cvtsi128_si32(_mm_cvtps_epi32(fmaxY));

      __m128 x1x0 = _mm_sub_ps(x1, x0);
      __m128 x2x1 = _mm_sub_ps(x2, x1);
      __m128 x0x2 = _mm_sub_ps(x0, x2);
      __m128 y1y0 = _mm_sub_ps(y1, y0);
      __m128 y2y0 = _mm_sub_ps(y2, y0);

      __m128 invArea = _mm_rcp_ps(_mm_sub_ps(_mm_mul_ps(_mm_sub_ps(x2, x0), y1y0), _mm_mul_ps(y2y0, x1x0)));

      __m128 z0 = _mm_set_ps1(v1[3]);
      __m128 invVert0 = _mm_mul_ps(z0, invArea);
      __m128 invVert1 = _mm_mul_ps(_mm_set_ps1(v2[3]), invArea);
      __m128 invVert2 = _mm_mul_ps(_mm_set_ps1(v3[3]), invArea);

      __m128 z1z0 = _mm_sub_ps(invVert1, invVert0);
      __m128 z2z0 = _mm_sub_ps(invVert2, invVert0);

      // We want the RGB values to be scaled by 255 in the end.
      // Doing that here saves us from having to apply the scale at each pixel.
      __m128 rgbScale = _mm_set_ps1(255.f);
      __m128 scaleFactor = _mm_mul_ps(invArea, rgbScale);

      __m128 r0 = _mm_set_ps1(v1[4]);
      __m128 invAttr00 = _mm_mul_ps(r0, scaleFactor);
      __m128 invAttr01 = _mm_mul_ps(_mm_set_ps1(v2[4]), scaleFactor);
      __m128 invAttr02 = _mm_mul_ps(_mm_set_ps1(v3[4]), scaleFactor);

      r0 = _mm_mul_ps(r0, rgbScale);
      __m128 r1r0 = _mm_sub_ps(invAttr01, invAttr00);
      __m128 r2r0 = _mm_sub_ps(invAttr02, invAttr00);

      __m128 g0 = _mm_set_ps1(v1[5]);
      __m128 invAttr10 = _mm_mul_ps(g0, scaleFactor);
      __m128 invAttr11 = _mm_mul_ps(_mm_set_ps1(v2[5]), scaleFactor);
      __m128 invAttr12 = _mm_mul_ps(_mm_set_ps1(v3[5]), scaleFactor);

      g0 = _mm_mul_ps(g0, rgbScale);
      __m128 g1g0 = _mm_sub_ps(invAttr11, invAttr10);
      __m128 g2g0 = _mm_sub_ps(invAttr12, invAttr10);

      __m128 b0 = _mm_set_ps1(v1[6]);
      __m128 invAttr20 = _mm_mul_ps(b0, scaleFactor);
      __m128 invAttr21 = _mm_mul_ps(_mm_set_ps1(v2[6]), scaleFactor);
      __m128 invAttr22 = _mm_mul_ps(_mm_set_ps1(v3[6]), scaleFactor);

      b0 = _mm_mul_ps(b0, rgbScale);
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

      __m128 initMultiplier = _mm_set_ps(3.f, 2.f, 1.f, 0.f);
      __m128 stepMultiplier = _mm_set_ps1(4.f);

      weightInit0 = _mm_sub_ps(weightInit0, _mm_mul_ps(initMultiplier, xStep0));
      weightInit1 = _mm_sub_ps(weightInit1, _mm_mul_ps(initMultiplier, xStep1));
      weightInit2 = _mm_sub_ps(weightInit2, _mm_mul_ps(initMultiplier, xStep2));

      xStep0 = _mm_mul_ps(stepMultiplier, xStep0);
      xStep1 = _mm_mul_ps(stepMultiplier, xStep1);
      xStep2 = _mm_mul_ps(stepMultiplier, xStep2);

      __m128i initialColor = _mm_set1_epi32(0xFF00);

      for (int32 h = minY; h < maxY; ++h) {
        __m128 weights0 = weightInit0;
        __m128 weights1 = weightInit1;
        __m128 weights2 = weightInit2;

        uint32* pixels = ((uint32*)(framebuffer)) + (minX + (h * sMaxWidth));
        float* pixelDepth = depthBuffer + (minX + (h * sMaxWidth));

        for (int32 w = minX; w < maxX; w += 4, pixels += 4, pixelDepth += 4) {
          // Fetch all sign bits and OR them together
          __m128 combinedWeights = _mm_or_ps(_mm_or_ps(weights0, weights1), weights2);
          int32 combinedMask = _mm_movemask_ps(combinedWeights);

          // If all mask bits are set then none of these pixels are inside the triangle.
          if (combinedMask != 0x0F) {
            // Our 4-wide alignment doesn't match at the moment. Possibly revisit in the future.
            __m128i pixelVec = _mm_loadu_si128((__m128i*)pixels);
            __m128 depthVec = _mm_loadu_ps(pixelDepth);

            __m128 weightedVerts0 = z0;
            __m128 weightedVerts1 = _mm_mul_ps(weights1, z1z0);
            __m128 weightedVerts2 = _mm_mul_ps(weights2, z2z0);

            __m128 zValues = _mm_add_ps(_mm_add_ps(weightedVerts0, weightedVerts1), weightedVerts2);
            __m128 invZValues = _mm_rcp_ps(zValues);

            __m128 depthMask = _mm_cmp_ps(zValues, depthVec, _CMP_GT_OQ);

            __m128i finalCombinedMask = _mm_castps_si128(_mm_or_ps(combinedWeights, depthMask));

            __m128 weightedAttr00 = _mm_mul_ps(r0, invZValues);
            __m128 weightedAttr01 = _mm_mul_ps(_mm_mul_ps(weights1, r1r0), invZValues);
            __m128 weightedAttr02 = _mm_mul_ps(_mm_mul_ps(weights2, r2r0), invZValues);

            __m128 weightedAttr10 = _mm_mul_ps(g0, invZValues);
            __m128 weightedAttr11 = _mm_mul_ps(_mm_mul_ps(weights1, g1g0), invZValues);
            __m128 weightedAttr12 = _mm_mul_ps(_mm_mul_ps(weights2, g2g0), invZValues);

            __m128 weightedAttr20 = _mm_mul_ps(b0, invZValues);
            __m128 weightedAttr21 = _mm_mul_ps(_mm_mul_ps(weights1, b1b0), invZValues);
            __m128 weightedAttr22 = _mm_mul_ps(_mm_mul_ps(weights2, b2b0), invZValues);

            __m128i rValues = _mm_cvtps_epi32(_mm_add_ps(_mm_add_ps(weightedAttr00, weightedAttr01), weightedAttr02));
            __m128i gValues = _mm_cvtps_epi32(_mm_add_ps(_mm_add_ps(weightedAttr10, weightedAttr11), weightedAttr12));
            __m128i bValues = _mm_cvtps_epi32(_mm_add_ps(_mm_add_ps(weightedAttr20, weightedAttr21), weightedAttr22));

            __m128i finalColor = initialColor;
            finalColor = _mm_or_si128(finalColor, rValues);
            finalColor = _mm_slli_epi32(finalColor, 0x08);
            finalColor = _mm_or_si128(finalColor, gValues);
            finalColor = _mm_slli_epi32(finalColor, 0x08);
            finalColor = _mm_or_si128(finalColor, bValues);

            __m128i writebackColor = _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(finalColor), _mm_castsi128_ps(pixelVec), _mm_castsi128_ps(finalCombinedMask)));
            __m128 writebackDepth = _mm_blendv_ps(zValues, depthVec, _mm_castsi128_ps(finalCombinedMask));

            _mm_storeu_si128((__m128i*)pixels, writebackColor);
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
}

void Unaligned_FlatShadeUVs(const float* vertices, const int32* indices, const int32 end, const float maxWidth, uint8* framebuffer, float* depthBuffer, const Texture* texture, size_t mipLevel) {
  const int32 sMaxWidth = (int32)maxWidth;
  // We want the UV values to be scaled by the width/height.
  // Doing that here saves us from having to do that at each pixel.
  // We must still multiply the stride and channels separately because of rounding error.
  size_t texWidth = texture->Width(mipLevel);
  size_t texHeight = texture->Height(mipLevel);
  uint32* textureData = (uint32*)texture->Data(mipLevel);
  
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    for (int32 i = 0; i < end; i += 3) {
      const float* v1 = vertices + indices[i];
      const float* v2 = vertices + indices[i + 1];
      const float* v3 = vertices + indices[i + 2];

      __m256 x0 = _mm256_broadcastss_ps(_mm_load_ps1(v1));
      __m256 x1 = _mm256_broadcastss_ps(_mm_load_ps1(v2));
      __m256 x2 = _mm256_broadcastss_ps(_mm_load_ps1(v3));

      __m256 y0 = _mm256_broadcastss_ps(_mm_load_ps1(v1 + 1));
      __m256 y1 = _mm256_broadcastss_ps(_mm_load_ps1(v2 + 1));
      __m256 y2 = _mm256_broadcastss_ps(_mm_load_ps1(v3 + 1));

      __m256 fminX = _mm256_min_ps(_mm256_min_ps(x0, x1), x2);
      __m256 fminY = _mm256_min_ps(_mm256_min_ps(y0, y1), y2);
      __m256 fmaxX = _mm256_max_ps(_mm256_max_ps(x0, x1), x2);
      __m256 fmaxY = _mm256_max_ps(_mm256_max_ps(y0, y1), y2);

#ifdef __clang__
      fminX = _mm256_round_ps(fminX, _MM_FROUND_FLOOR);
      fminY = _mm256_round_ps(fminY, _MM_FROUND_FLOOR);

      fmaxX = _mm256_round_ps(fmaxX, _MM_FROUND_CEIL);
      fmaxY = _mm256_round_ps(fmaxY, _MM_FROUND_CEIL);
#else
      fminX = _mm256_round_ps(fminX, _MM_ROUND_MODE_DOWN);
      fminY = _mm256_round_ps(fminY, _MM_ROUND_MODE_DOWN);

      fmaxX = _mm256_round_ps(fmaxX, _MM_ROUND_MODE_UP);
      fmaxY = _mm256_round_ps(fmaxY, _MM_ROUND_MODE_UP);
#endif

      int32 minX = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fminX));
      int32 maxX = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fmaxX));
      int32 minY = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fminY));
      int32 maxY = _mm256_cvtsi256_si32(_mm256_cvtps_epi32(fmaxY));

      __m256 x1x0 = _mm256_sub_ps(x1, x0);
      __m256 x2x1 = _mm256_sub_ps(x2, x1);
      __m256 x0x2 = _mm256_sub_ps(x0, x2);
      __m256 y1y0 = _mm256_sub_ps(y1, y0);
      __m256 y2y0 = _mm256_sub_ps(y2, y0);

      __m256 invArea = _mm256_rcp_ps(_mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(x2, x0), y1y0), _mm256_mul_ps(y2y0, x1x0)));
        
      __m256 z0 = _mm256_set1_ps(v1[3]);
      __m256 invVert0 = _mm256_mul_ps(z0, invArea);
      __m256 invVert1 = _mm256_mul_ps(_mm256_set1_ps(v2[3]), invArea);
      __m256 invVert2 = _mm256_mul_ps(_mm256_set1_ps(v3[3]), invArea);

      __m256 z1z0 = _mm256_sub_ps(invVert1, invVert0);
      __m256 z2z0 = _mm256_sub_ps(invVert2, invVert0);

      __m256 yStride = _mm256_set1_ps((float)(texHeight));

      __m256 maxUValue = _mm256_set1_ps((float)(texWidth - 1));
      __m256 maxVValue = _mm256_set1_ps((float)(texHeight - 1));

      __m256 uScaleFactor = _mm256_mul_ps(invArea, maxUValue);
      __m256 vScaleFactor = _mm256_mul_ps(invArea, maxVValue);

      __m256 u0 = _mm256_set1_ps(v1[4]);
      __m256 invAttr00 = _mm256_mul_ps(u0, uScaleFactor);
      __m256 invAttr01 = _mm256_mul_ps(_mm256_set1_ps(v2[4]), uScaleFactor);
      __m256 invAttr02 = _mm256_mul_ps(_mm256_set1_ps(v3[4]), uScaleFactor);
      u0 = _mm256_mul_ps(u0, maxUValue);
      __m256 u1u0 = _mm256_sub_ps(invAttr01, invAttr00);
      __m256 u2u0 = _mm256_sub_ps(invAttr02, invAttr00);

      __m256 v0 = _mm256_set1_ps(v1[5]);
      __m256 invAttr10 = _mm256_mul_ps(v0, vScaleFactor);
      __m256 invAttr11 = _mm256_mul_ps(_mm256_set1_ps(v2[5]), vScaleFactor);
      __m256 invAttr12 = _mm256_mul_ps(_mm256_set1_ps(v3[5]), vScaleFactor);
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
      __m256 weightInit0 = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(fminX, x1), _mm256_sub_ps(y2, y1)), _mm256_mul_ps(_mm256_sub_ps(fminY, y1), x2x1));
      __m256 weightInit1 = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(fminX, x2), _mm256_sub_ps(y0, y2)), _mm256_mul_ps(_mm256_sub_ps(fminY, y2), x0x2));
      __m256 weightInit2 = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(fminX, x0), y1y0), _mm256_mul_ps(_mm256_sub_ps(fminY, y0), x1x0));

      __m256 xStep0 = _mm256_sub_ps(y1, y2);
      __m256 xStep1 = y2y0;
      __m256 xStep2 = _mm256_sub_ps(y0, y1);

      __m256 yStep0 = x2x1;
      __m256 yStep1 = x0x2;
      __m256 yStep2 = x1x0;

      __m256 initMultiplier = _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f);
      __m256 stepMultiplier = _mm256_set1_ps(8.f);

      weightInit0 = _mm256_sub_ps(weightInit0, _mm256_mul_ps(initMultiplier, xStep0));
      weightInit1 = _mm256_sub_ps(weightInit1, _mm256_mul_ps(initMultiplier, xStep1));
      weightInit2 = _mm256_sub_ps(weightInit2, _mm256_mul_ps(initMultiplier, xStep2));

      xStep0 = _mm256_mul_ps(stepMultiplier, xStep0);
      xStep1 = _mm256_mul_ps(stepMultiplier, xStep1);
      xStep2 = _mm256_mul_ps(stepMultiplier, xStep2);

      __m256i weightMask = _mm256_set1_epi32(0x80000000);

      for (int32 h = minY; h < maxY; ++h) {
        __m256 weights0 = weightInit0;
        __m256 weights1 = weightInit1;
        __m256 weights2 = weightInit2;

        uint32* pixels = ((uint32*)(framebuffer)) + (minX + (h * sMaxWidth));
        float* pixelDepth = depthBuffer + (minX + (h * sMaxWidth));

        for (int32 w = minX; w < maxX; w += 8, pixels += 8, pixelDepth += 8) {
          // OR all weights and fetch the sign bits
          __m256 combinedWeights = _mm256_or_ps(_mm256_or_ps(weights0, weights1), weights2);

          // If all mask bits are set then none of these pixels are inside the triangle.
          if (!_mm256_testc_si256(_mm256_castps_si256(combinedWeights), weightMask)) {
            __m256 depthVec = _mm256_loadu_ps(pixelDepth);

            __m256 zValues = _mm256_fmadd_ps(weights2, z2z0, _mm256_fmadd_ps(weights1, z1z0, z0));

            __m256 invZValues = _mm256_rcp_ps(zValues);

            __m256 depthMask = _mm256_cmp_ps(zValues, depthVec, _CMP_GT_OQ);

            __m256i finalCombinedMask = Not256(_mm256_castps_si256(_mm256_or_ps(combinedWeights, depthMask)));

            __m256 uValues = _mm256_fmadd_ps(_mm256_mul_ps(weights2, u2u0), invZValues, _mm256_fmadd_ps(u0, invZValues, _mm256_mul_ps(_mm256_mul_ps(weights1, u1u0), invZValues)));
            __m256 vValues = _mm256_fmadd_ps(_mm256_mul_ps(weights2, v2v0), invZValues, _mm256_fmadd_ps(v0, invZValues, _mm256_mul_ps(_mm256_mul_ps(weights1, v1v0), invZValues)));

            // We must round prior to multiplying the stride and channels.
            // If this isn't done, we may jump to a completely different set of pixels because of rounding.
#ifdef __clang__
            vValues = _mm256_round_ps(vValues, _MM_FROUND_FLOOR);
#else
            vValues = _mm256_round_ps(vValues, _MM_ROUND_MODE_DOWN);
#endif

            __m256i colorValues = _mm256_cvtps_epi32(_mm256_fmadd_ps(vValues, yStride, uValues));

            // Note: we're assuming texture data is stored in ARGB format.
            // If it isn't, we need to shuffle and handle alpha here as well.
            // This can be handled outside of the render loop in the texture loading code.

            // NOTE: Gathers are faster in the general case except on some early HW that didn't optimize for it!
            //  If we plan on optimizing for all cases, we will need to take this into account.
            //  This memory read is by far the biggest bottleneck here.
            __m256i loadedColors = _mm256_mask_i32gather_epi32(_mm256_setzero_si256(), (const int*)textureData, colorValues, finalCombinedMask, 4);

            _mm256_maskstore_epi32((int*)pixels, finalCombinedMask, loadedColors);
            _mm256_maskstore_ps(pixelDepth, finalCombinedMask, zValues);
          }

          weights0 = _mm256_sub_ps(weights0, xStep0);
          weights1 = _mm256_sub_ps(weights1, xStep1);
          weights2 = _mm256_sub_ps(weights2, xStep2);
        }

        weightInit0 = _mm256_sub_ps(weightInit0, yStep0);
        weightInit1 = _mm256_sub_ps(weightInit1, yStep1);
        weightInit2 = _mm256_sub_ps(weightInit2, yStep2);
      }
    }
  }
  else {
    for (int32 i = 0; i < end; i += 3) {
      const float* v1 = vertices + indices[i];
      const float* v2 = vertices + indices[i + 1];
      const float* v3 = vertices + indices[i + 2];

      __m128 x0 = _mm_load_ps1(v1);
      __m128 x1 = _mm_load_ps1(v2);
      __m128 x2 = _mm_load_ps1(v3);

      __m128 y0 = _mm_load_ps1(v1 + 1);
      __m128 y1 = _mm_load_ps1(v2 + 1);
      __m128 y2 = _mm_load_ps1(v3 + 1);

      __m128 fminX = _mm_min_ps(_mm_min_ps(x0, x1), x2);
      __m128 fminY = _mm_min_ps(_mm_min_ps(y0, y1), y2);
      __m128 fmaxX = _mm_max_ps(_mm_max_ps(x0, x1), x2);
      __m128 fmaxY = _mm_max_ps(_mm_max_ps(y0, y1), y2);

      fminX = _mm_round_ps(fminX, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
      fminY = _mm_round_ps(fminY, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);

      fmaxX = _mm_round_ps(fmaxX, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);
      fmaxY = _mm_round_ps(fmaxY, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);

      int32 minX = _mm_cvtsi128_si32(_mm_cvtps_epi32(fminX));
      int32 maxX = _mm_cvtsi128_si32(_mm_cvtps_epi32(fmaxX));
      int32 minY = _mm_cvtsi128_si32(_mm_cvtps_epi32(fminY));
      int32 maxY = _mm_cvtsi128_si32(_mm_cvtps_epi32(fmaxY));

      __m128 x1x0 = _mm_sub_ps(x1, x0);
      __m128 x2x1 = _mm_sub_ps(x2, x1);
      __m128 x0x2 = _mm_sub_ps(x0, x2);
      __m128 y1y0 = _mm_sub_ps(y1, y0);
      __m128 y2y0 = _mm_sub_ps(y2, y0);

      __m128 invArea = _mm_rcp_ps(_mm_sub_ps(_mm_mul_ps(_mm_sub_ps(x2, x0), y1y0), _mm_mul_ps(y2y0, x1x0)));

      __m128 z0 = _mm_set_ps1(v1[3]);
      __m128 invVert0 = _mm_mul_ps(z0, invArea);
      __m128 invVert1 = _mm_mul_ps(_mm_set_ps1(v2[3]), invArea);
      __m128 invVert2 = _mm_mul_ps(_mm_set_ps1(v3[3]), invArea);

      __m128 z1z0 = _mm_sub_ps(invVert1, invVert0);
      __m128 z2z0 = _mm_sub_ps(invVert2, invVert0);

      __m128 yStride = _mm_set_ps1((float)(texHeight));

      __m128 maxUValue = _mm_set_ps1((float)(texWidth - 1));
      __m128 maxVValue = _mm_set_ps1((float)(texHeight - 1));

      __m128 uScaleFactor = _mm_mul_ps(invArea, maxUValue);
      __m128 vScaleFactor = _mm_mul_ps(invArea, maxVValue);

      __m128 u0 = _mm_set_ps1(v1[4]);
      __m128 invAttr00 = _mm_mul_ps(u0, uScaleFactor);
      __m128 invAttr01 = _mm_mul_ps(_mm_set_ps1(v2[4]), uScaleFactor);
      __m128 invAttr02 = _mm_mul_ps(_mm_set_ps1(v3[4]), uScaleFactor);
      u0 = _mm_mul_ps(u0, maxUValue);
      __m128 u1u0 = _mm_sub_ps(invAttr01, invAttr00);
      __m128 u2u0 = _mm_sub_ps(invAttr02, invAttr00);

      __m128 v0 = _mm_set_ps1(v1[5]);
      __m128 invAttr10 = _mm_mul_ps(v0, vScaleFactor);
      __m128 invAttr11 = _mm_mul_ps(_mm_set_ps1(v2[5]), vScaleFactor);
      __m128 invAttr12 = _mm_mul_ps(_mm_set_ps1(v3[5]), vScaleFactor);
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

      __m128 initMultiplier = _mm_set_ps(3.f, 2.f, 1.f, 0.f);
      __m128 stepMultiplier = _mm_set_ps1(4.f);

      weightInit0 = _mm_sub_ps(weightInit0, _mm_mul_ps(initMultiplier, xStep0));
      weightInit1 = _mm_sub_ps(weightInit1, _mm_mul_ps(initMultiplier, xStep1));
      weightInit2 = _mm_sub_ps(weightInit2, _mm_mul_ps(initMultiplier, xStep2));

      xStep0 = _mm_mul_ps(stepMultiplier, xStep0);
      xStep1 = _mm_mul_ps(stepMultiplier, xStep1);
      xStep2 = _mm_mul_ps(stepMultiplier, xStep2);

      __m128 minValue = _mm_setzero_ps();

      for (int32 h = minY; h < maxY; ++h) {
        __m128 weights0 = weightInit0;
        __m128 weights1 = weightInit1;
        __m128 weights2 = weightInit2;

        uint32* pixels = ((uint32*)(framebuffer)) + (minX + (h * sMaxWidth));
        float* pixelDepth = depthBuffer + (minX + (h * sMaxWidth));

        for (int32 w = minX; w < maxX; w += 4, pixels += 4, pixelDepth += 4) {
          // Fetch all sign bits and OR them together
          __m128 combinedWeights = _mm_or_ps(_mm_or_ps(weights0, weights1), weights2);
          int32 combinedMask = _mm_movemask_ps(combinedWeights);

          // If all mask bits are set then none of these pixels are inside the triangle.
          if ((combinedMask & 0x0F) != 0x0F) {
            // Our 4-wide alignment doesn't match at the moment. Possibly revisit in the future.
            __m128i pixelVec = _mm_loadu_si128((__m128i*)pixels);
            __m128 depthVec = _mm_loadu_ps(pixelDepth);

            __m128 weightedVerts0 = z0;
            __m128 weightedVerts1 = _mm_mul_ps(weights1, z1z0);
            __m128 weightedVerts2 = _mm_mul_ps(weights2, z2z0);

            __m128 zValues = _mm_add_ps(_mm_add_ps(weightedVerts0, weightedVerts1), weightedVerts2);
            __m128 invZValues = _mm_rcp_ps(zValues);

            __m128 depthMask = _mm_cmp_ps(zValues, depthVec, _CMP_GT_OQ);

            __m128i finalCombinedMask = _mm_castps_si128(_mm_or_ps(combinedWeights, depthMask));

            __m128 weightedAttr00 = _mm_mul_ps(u0, invZValues);
            __m128 weightedAttr01 = _mm_mul_ps(_mm_mul_ps(weights1, u1u0), invZValues);
            __m128 weightedAttr02 = _mm_mul_ps(_mm_mul_ps(weights2, u2u0), invZValues);

            __m128 weightedAttr10 = _mm_mul_ps(v0, invZValues);
            __m128 weightedAttr11 = _mm_mul_ps(_mm_mul_ps(weights1, v1v0), invZValues);
            __m128 weightedAttr12 = _mm_mul_ps(_mm_mul_ps(weights2, v2v0), invZValues);

            __m128 uValues = _mm_add_ps(_mm_add_ps(weightedAttr00, weightedAttr01), weightedAttr02);
            __m128 vValues = _mm_add_ps(_mm_add_ps(weightedAttr10, weightedAttr11), weightedAttr12);

            // Clamp UV so that we don't index outside of the texture.
            uValues = _mm_min_ps(uValues, maxUValue);
            uValues = _mm_max_ps(uValues, minValue);
            vValues = _mm_min_ps(vValues, maxVValue);
            vValues = _mm_max_ps(vValues, minValue);

            // We must round prior to multiplying the stride and channels.
            // If this isn't done, we may jump to a completely different set of pixels because of rounding.
#ifdef __clang__
            vValues = _mm_round_ps(vValues, _MM_FROUND_FLOOR);
#else
            vValues = _mm_round_ps(vValues, _MM_ROUND_MODE_DOWN);
#endif

            vValues = _mm_add_ps(_mm_mul_ps(vValues, yStride), uValues);

            __m128i colorValues = _mm_cvtps_epi32(vValues);

            __m128i loadedColors = _mm_set_epi32(textureData[_mm_extract_epi32(colorValues, 0b11)], textureData[_mm_extract_epi32(colorValues, 0b10)], textureData[_mm_extract_epi32(colorValues, 0b01)], textureData[_mm_extract_epi32(colorValues, 0b00)]);

            __m128i writebackColor = _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(loadedColors), _mm_castsi128_ps(pixelVec), _mm_castsi128_ps(finalCombinedMask)));
            __m128 writebackDepth = _mm_blendv_ps(zValues, depthVec, _mm_castsi128_ps(finalCombinedMask));

            _mm_storeu_si128((__m128i*)pixels, writebackColor);
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
}

}

#endif

#endif
