#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZFile.h"

namespace ZSharp {

class MP3 final {
  public:

  struct PCMAudio {
    int16* data = nullptr;
    size_t length = 0;
    size_t channels = 0;
    size_t samplesPerSecond = 0;
  };

  MP3(const FileString& path);

  PCMAudio Decompress();

  private:
  MemoryMappedFileReader mReader;

  uint8* mDataStream = nullptr;
  size_t mOffset = 0;
  size_t mBitOffset = 0;
  size_t mStreamSize = 0;

  enum class ChannelMode {
    Stereo,
    JointStero,
    DualChannel,
    SingleChannel,
    Undefined
  };

  enum class ChannelModeExt {
    MidStereo,
    IntensityStereo,
    Both,
    Undefined
  };

  enum class SideBlockType {
    StartBlock,
    ShortWindows,
    End,
    Undefined
  };

  struct FrameHeader {
    ChannelMode mode = ChannelMode::Undefined;
    ChannelModeExt modeExt = ChannelModeExt::Undefined;
    uint32 bitrate = 0;
    double sampleRate = 0.; // Can be a rational number.
    uint32 sampleRateBits = 0;
    size_t frameBytes = 0;
  };

  struct Granule {
    size_t scaleFactorSelect = 0;
    size_t part2_3_length = 0;
    size_t bigValues = 0;
    size_t globalGain = 0;
    size_t scaleFactorCompress = 0;
    const uint8* scaleFactorTable = nullptr;
    size_t shortScaleBand = 0;
    size_t longScaleBand = 0;
    bool windowSwitch = false;
    SideBlockType blockType = SideBlockType::Undefined;
    bool mixedBlocks = false;
    size_t huffmanTables[3] = {};
    size_t subBlockGain[3] = {};
    size_t regions[3] = {};
    uint32 preflag = 0;
    uint32 scalefactorScale = 0;
    uint32 countTableSelect = 0;
  };

  struct SideInfomation {
    size_t byteLength = 0;
    size_t numChannels = 2;
    size_t mainData = 0;
    Granule gr0[2];
    Granule gr1[2];
  };

  enum Endian {
    Big,
    Little
  };

  uint32 ReadBits(uint8* buffer, size_t& bitOffset, size_t count, Endian endian);

  bool ReadFileHeader();

  FrameHeader ReadFrameHeader();

  SideInfomation ReadSideInformation(uint32 sampleRateBits, ChannelMode mode, ChannelModeExt ext);

  void DecodeMainData(uint8* resevoir, 
    size_t& resevoirBitsRead, 
    SideInfomation& sideInfo, 
    ChannelModeExt ext, 
    size_t sampleRateBits, 
    float* intermediateValues, 
    float* overlapValues,
    int16* outData,
    float* qmfState);

  void DecodeScaleFactors(uint8* resevoir, size_t& resevoirBitsRead, Granule& granule, size_t scaleFactorSelect, bool isMidStereo, float* scaleFactors, uint8* sharedScaleFactors);

  static void DecodeHuffman(uint8* resevoir, size_t& resevoirBitsRead, Granule& granule, float* scaleFactors, float* intermediateValues, size_t frameEnd);

  static float DecodeExponent(float y, int32 exponent);

  static ChannelMode ChannelFromMode(size_t flags);

  static ChannelModeExt ChannelExtFromModeExt(size_t flags);

  static size_t BitrateFromFlagsL3(size_t flags);

  static double SampleRateFromFrequencyL3(size_t frequency);

  static float Power43(int32 x);

  static void MidStereoProcess(float* left, size_t length);

  static void IntensityStereoProcess(float* left, uint8* sharedScaleFactors, Granule& granule);

  static void StereoTopBand(const float* right, const uint8* sharedScaleFactors, int32 numBands, int* maxBands);

  static void StereoProcess(float* left, const uint8* sharedScaleFactors, const uint8* scaleFactorBands, int* maxBands);

  static void Reorder(float* buffer, float* scratch, const uint8* scalefactors);

  static void AntiAlias(float* buffer, int32 bands);

  static void InverseMDCTGranule(float* buffer, float* overlap, SideBlockType blockType, int32 bands);

  static void InverseMDCT36(float* buffer, float* overlap, const float* window, int32 bands);

  static void InverseMDCTShort(float* buffer, float* overlap, int32 bands);

  static void InverseMDCT12(float* x, float* dest, float* overlap);

  static void InverseMDCT3(float x0, float x1, float x2, float* dest);

  static void DCT2(float* buffer, int32 n);

  static void DCT39(float* y);

  static void FlipSigns(float* buffer);

  static void SynthesizeGranule(float* buffer, int32 bands, size_t channels, int16* outData, float* lines);

  static void SynthesizePair(int16* outData, size_t channels, const float* z);

  static void Synthesize(float* x1, int16* outData, size_t channels, float* lines);

  static int16 ScalePCM(float sample);
};

}
