#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZFile.h"

namespace ZSharp {

class PNG final {
  public:

  PNG(const FileString& filename);
  PNG(const PNG& rhs) = delete;
  ~PNG();

  // TODO: This should read from the PNG, allocate a buffer large enough, and decompress into that buffer.
  // The caller will have a pointer which they are responsible for managing the lifetime of.
  bool Decompress(uint8* buffer);

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
    uint16 lengthCodes[16];
    uint16 symbols[288];
    uint16 distanceCodes[16];
    uint16 distanceSymbols[30];
  };

  const uint8* DataOffset();

  const uint8* DataBitOffset();

  bool ReadHeader();

  void IdentifyChunk(uint32& length, uint32& type);

  uint32 ReadBits(size_t count);

  void DecodeDynamicHuffman(uint8* output);

  void BuildHuffman(PngHuffman& huffman, uint8* lengths, int32 count);
};

}
