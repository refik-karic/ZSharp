#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZFile.h"

namespace ZSharp {

enum class ChannelOrder : size_t {
  RGB, // Native PNG
  BGR
};

class PNG final {
  public:

  PNG(const FileString& filename);
  PNG(const PNG& rhs) = delete;
  ~PNG();

  // Allocate and return a buffer to the caller.
  // NOTE: The RGB channels may not be arranged in a native way for the current display.
  //  This means we may need to do some channel swapping to get it right.
  uint8* Decompress(ChannelOrder order);

  size_t GetWidth() const;

  size_t GetHeight() const;

  size_t GetBitsPerPixel() const;

  private:
  const FileString mFilename;
  MemoryMappedFileReader mReader;

  size_t mWidth = 0;
  size_t mHeight = 0;
  size_t mStride = 0;
  size_t mBitsPerPixel = 0;
  size_t mChannels = 0;
  size_t mDataOffset = 0; // Used to index into the file data.
  size_t mBitOffset = 0; // Used to index into the Deflate stream.

  struct PngHuffman {
    uint16 codes[16];
    uint16 symbols[286];
  };

  enum PngFilter {
    None = 0,
    Sub = 1,
    Up = 2,
    Average = 3,
    Paeth = 4
  };

  uint8 PaethPredictor(int64 left, int64 above, int64 aboveLeft);

  const uint8* DataOffset();

  const uint8* DataBitOffset();

  bool ReadHeader();

  void IdentifyChunk(uint32& length, uint32& type);

  uint32 ReadBits(size_t count);

  bool DecodeDynamicHuffman(uint8* output);

  bool BuildHuffman(PngHuffman& huffman, uint16* lengths, int32 count);

  bool DecodeSymbol(const PngHuffman& huffman, uint32& symbol);

  bool DeflateStream(uint8* output, const PngHuffman& lengthHuff, const PngHuffman& distHuff);

  uint8* FilterDeflatedImage(uint8* image);
};

}
