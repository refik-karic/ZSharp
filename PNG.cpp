#include "PNG.h"

#include "ZAssert.h"
#include "Common.h"
#include "CommonMath.h"

#include "PlatformMemory.h"

#include <cstring>

// See: https://en.wikipedia.org/wiki/PNG
// Also: http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
// RFC1950 (zlib): https://www.rfc-editor.org/rfc/rfc1950.txt
//  This contains the zlib header spec.
//  The zlib header is the first thing in the IDAT chunk.
//  The DEFLATE stream is contained after the zlib header.
// RFC1951 (DEFLATE): https://www.ietf.org/rfc/rfc1951.txt
//  Has a very thorough explanation of DEFLATE and Huffman Trees with code examples.

static const size_t MaxBits = 15;

namespace ZSharp {
PNG::PNG() 
  : mFilename(""), mReader(FileString("")) {
}

PNG::PNG(const FileString& filename)
  : mFilename(filename), mReader(filename) {
  if (mReader.IsOpen()) {
    mDataPtr = (uint8*)mReader.GetBuffer();
    mFileSize = mReader.GetSize();
    ReadHeader();
  }
}

PNG::~PNG() {
}

void PNG::Serialize(MemorySerializer& serializer) {
  // Restrict this to serializing PNG files to bundles.
  if (!mReader.IsOpen()) {
    ZAssert(false);
    return;
  }

  serializer.Serialize(&mFileSize, sizeof(mFileSize));
  serializer.Serialize(mDataPtr, mFileSize);
}

void PNG::Deserialize(MemoryDeserializer& deserializer) {
  // Only deserialize an unbound PNG.
  if (mReader.IsOpen()) {
    ZAssert(false);
    return;
  }

  size_t fileSize = 0;
  if (!deserializer.Deserialize(&fileSize, sizeof(fileSize))) {
    ZAssert(false);
    return;
  }

  const size_t padding = sizeof(size_t);

  mFileSize = fileSize;
  mDataPtr = deserializer.BaseAddress() + deserializer.Offset() + padding;

  ReadHeader();
}

uint8* PNG::Decompress(ChannelOrder order) {
  if (mDataPtr == nullptr) {
    return nullptr;
  }

  /*
  TODO: This is not ideal. We allocate/reallocate length sized chunks into one big IDAT chunk.
        Long term we want skip over chunks properly without doing this.
  */
  uint8* chunkedIDATData = nullptr;
  size_t chunkedIDATDataLength = 0;

  const size_t CRCLength = 4;
  for (bool foundEndChunk = false; !foundEndChunk;) {
    uint32 length = 0;
    uint32 id = 0;
    IdentifyChunk(length, id);

    if (!memcmp(&id, "IDAT", 4)) {
      if (chunkedIDATData == nullptr) {
        chunkedIDATData = (uint8*)PlatformMalloc(length);
        chunkedIDATDataLength = (size_t)length;
        memcpy(chunkedIDATData, DataOffset(), length);
      }
      else {
        const size_t oldLength = chunkedIDATDataLength;
        chunkedIDATDataLength += (size_t)length;
        chunkedIDATData = (uint8*)PlatformReAlloc(chunkedIDATData, chunkedIDATDataLength);
        memcpy(chunkedIDATData + oldLength, DataOffset(), length);
      }
    }
    else if (!memcmp(&id, "IEND", 4)) {
      foundEndChunk = true;
    }
    
    mDataOffset += length;
    mDataOffset += CRCLength;
  }

  if (chunkedIDATData == nullptr) {
    ZAssert(false);
    return nullptr;
  }

  /*
  NOTE: PNG spec is a little confusing when it comes to multiple IDATs
        It is perfectly legal to have more than 1 but they should be treated as one big stream.
        This means we only check the zlib header once.
        The rest of the data should be treated as one big DEFLATE stream.
        This means:
          1) Check for end of stream
          2) Determine compression type
          3) Repeat until end of stream
  */

  mChunkedIDATData = chunkedIDATData;
  mBitOffset = 0;

  // Sanity check for zlib header DEFLATE compression method.
  if ((mChunkedIDATData[0] & 0x0F) != 8) {
    ZAssert(false);
    return nullptr;
  }

  // Skip over first two zlib header bytes.
  // PNG spec does not allow the optional preset dictionary so we skip directly to the DEFLATE stream.
  mChunkedIDATData += 2;

  // PNG spec calls for a filter byte on each scan line before/after compression.
  // This denotes one of 5 filtering methods that help to reduce image size before DEFLATE is applied.
  const size_t decompressedSize = ((mStride + 1) * mHeight);
  uint8* buffer = (uint8*)PlatformMalloc(decompressedSize);

  for (bool reading = true; reading;) {
    uint32 isFinal = ReadBits(1);
    reading = (isFinal == 0);

    uint32 compressedType = ReadBits(2);
    if (compressedType == 0) {
      PlatformFree(buffer);
      ZAssert(false);
      return nullptr;
#if 0
      // TODO: This is a "stored"/uncompressed block.
      //  2 next bytes are LEN
      //  2 next bytes are NLEN (ones-complement of previous length)
      //  Next LEN bytes are uncompressed data that should be copied to the output.
      mBitOffset = RoundUpNearestMultiple(mBitOffset, 8);

      const uint8* data = DataBitOffset();

      uint16 uncompressedLength = data[0];
      uncompressedLength |= (((uint16)(data[1])) << 8);
      uint16 onesCompLength = data[2];
      onesCompLength |= (((uint16)(data[3])) << 8);

      if ((uncompressedLength) != (~onesCompLength)) {
        ZAssert(false);
        return nullptr;
      }

      mBitOffset += 32;
      mBitOffset += (uncompressedLength * 8);

      memcpy(buffer, data + 4, uncompressedLength);
      outputBufferPos += uncompressedLength;
#endif
    }
    else if (compressedType == 2) {
      if (!DecodeDynamicHuffman(buffer)) {
        PlatformFree(buffer);
        ZAssert(false);
        return nullptr;
      }
    }
  }

  PlatformFree(chunkedIDATData);
  chunkedIDATData = nullptr;
  mChunkedIDATData = nullptr;
  mBitOffset = 0;

  uint8* outputImage = FilterDeflatedImage(buffer);

  switch (order) {
    case ChannelOrder::RGB: // Native order per spec.
      break;
    case ChannelOrder::BGR:
      for (size_t y = 0; y < mHeight; ++y) {
        for (size_t x = 0; x < mStride; x+=mChannels) {
          const size_t redChannel = (y * mStride) + x;
          const size_t blueChannel = (y * mStride) + x + 2;
          uint8 blueValue = outputImage[blueChannel];
          uint8 redValue = outputImage[redChannel];
          outputImage[blueChannel] = redValue;
          outputImage[redChannel] = blueValue;
        }
      }
      break;
  }

  return outputImage;
}

size_t PNG::GetWidth() const {
  return mWidth;
}

size_t PNG::GetHeight() const {
  return mHeight;
}

size_t PNG::GetNumChannels() const {
  return mChannels;
}

size_t PNG::GetBitsPerPixel() const {
  return mBitsPerPixel;
}

bool PNG::ReadHeader() {
  if (mDataPtr == nullptr) {
    ZAssert(false);
    return false;
  }

  const size_t MagicHeaderLength = 8;
  const size_t CRCLength = 4;

  const uint8* data = reinterpret_cast<const uint8*>(mDataPtr);

  char header[MagicHeaderLength];
  memset(header, 0, sizeof(header));
  memcpy(header, data, sizeof(header));

  mDataOffset += MagicHeaderLength;
  data += MagicHeaderLength;

  // Next is the first chunk.
  // This should be the "IHDR" chunk which contains information about the image.
  // NOTE: All chunks are in network order (Big Endian).

  uint32 chunkLength = 0;
  uint32 chunkID = 0;

  IdentifyChunk(chunkLength, chunkID);
  
  if (memcmp(&chunkID, "IHDR", 4)) {
    ZAssert(false);
    return false;
  }

  data += 8;

  uint32 width = EndianSwap(*((uint32*)data));
  uint32 height = EndianSwap(*((uint32*)(data + 4)));
  uint8 bitDepth = *((uint8*)(data + 8));
  uint8 colorType = *((uint8*)(data + 9));
  uint8 compressionMethod = *((uint8*)(data + 10));
  uint8 filterMethod = *((uint8*)(data + 11));
  uint8 interlaceMethod = *((uint8*)(data + 12));
  (void)compressionMethod;
  (void)filterMethod;
  (void)interlaceMethod;

  mDataOffset += chunkLength;
  mDataOffset += CRCLength;

  if (colorType == 2) {
    // Indicates RGB triple.
    // Spec requires 8 or 16 bits per pixel for triples.
    if (bitDepth != 8 && bitDepth != 16) {
      ZAssert(false);
      return false;
    }

    mWidth = width;
    mHeight = height;
    mStride = width * 3;
    mBitsPerPixel = bitDepth * 3;
    mChannels = 3;
  }
  else if (colorType == 6) {
    // Indicates RGBA triple.
    if (bitDepth != 8 && bitDepth != 16) {
      ZAssert(false);
      return false;
    }

    mWidth = width;
    mHeight = height;
    mStride = width * 4;
    mBitsPerPixel = bitDepth * 4;
    mChannels = 4;
  }
  else {
    // Implement other formats as necessary.
    ZAssert(false);
    return false;
  }

  for (bool foundDataChunk = false; !foundDataChunk;) {
    uint32 length = 0;
    uint32 id = 0;
    IdentifyChunk(length, id);

    if (!memcmp(&id, "IDAT", 4)) {
      foundDataChunk = true;
      mDataOffset -= 8; // Seek back to the start of chunk for decoding.
    }
    else if(!memcmp(&id, "IEND", 4)) {
      // Empty file with no IDAT chunk.
      ZAssert(false);
      return false;
    }
    else {
      mDataOffset += length;
      mDataOffset += CRCLength;
    }

    if (mDataOffset >= mFileSize) {
      ZAssert(false);
      return false;
    }
  }

  return true;
}

int64 PNG::PaethPredictor(int64 left, int64 above, int64 aboveLeft) {
  int64 p = left + above - aboveLeft;
  int64 pa = llabs(p - left);
  int64 pb = llabs(p - above);
  int64 pc = llabs(p - aboveLeft);
  if ((pa <= pb) && (pa <= pc)) {
    return left;
  }
  else if (pb <= pc) {
    return above;
  }
  else {
    return aboveLeft;
  }
}

const uint8* PNG::DataOffset() {
  return reinterpret_cast<const uint8*>(mDataPtr) + mDataOffset;
}

const uint8* PNG::DataBitOffset() {
  return reinterpret_cast<const uint8*>(mDataPtr) + mDataOffset + (mBitOffset / 8);
}

uint32 PNG::ReadBits(size_t count) {
  uint32 ret = 0;

  if (count > MaxBits) {
    ZAssert(false);
    return 0;
  }
  else if (count == 0) {
    return 0;
  }

  const uint8* data = (mChunkedIDATData + (mBitOffset / 8));
  const size_t bitsReadInByte = (mBitOffset % 8);

  // Read the first bytes, drop unneeded bits.
  size_t bytesRead = 0;
  ret |= data[bytesRead];

  for (size_t i = (8 - bitsReadInByte); i < count; i += 8) {
    ++bytesRead;
    const uint32 shiftAmount = ((uint32)bytesRead * 8);
    ZAssert(shiftAmount < 32);
    ret |= (((uint32)data[bytesRead]) << shiftAmount);
  }

  ret >>= bitsReadInByte;
  ret &= ((1U << count) - 1U);

  mBitOffset += count;
  return ret;
}

void PNG::IdentifyChunk(uint32& length, uint32& type) {
  const uint8* data = DataOffset();

  length = EndianSwap(*((uint32*)data));
  type = *((uint32*)(data + 4)); // Chunk type is not Big Endian.

  mDataOffset += 8;
}

bool PNG::DecodeDynamicHuffman(uint8* output) {
  /*
  Steps:
    1) Decode Huffman tree for lengths.
    2) Build the length and distance Huffman Trees using the decoded data.
    3) Walk the stream and decode symbols using the decoded Huffman Trees.
  */

  const uint32 lengthCodes = ReadBits(5) + 257;
  const uint32 distCodes = ReadBits(5) + 1;
  const uint32 codeCodes = ReadBits(4) + 4;

  if (lengthCodes > 286 || distCodes > 32 || codeCodes > 19) {
    ZAssert(false);
    return false;
  }

  uint16 alphabet[286 + 30];
  memset(alphabet, 0, sizeof(alphabet));
  const uint8 alphabetOrder[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

  for (uint32 i = 0; i < codeCodes; ++i) {
    uint32 symbol = ReadBits(3);
    alphabet[alphabetOrder[i]] = static_cast<uint16>(symbol);
  }

  PngHuffman lengthHuffman;
  memset(&lengthHuffman, 0, sizeof(PngHuffman));
  if (!BuildHuffman(lengthHuffman, alphabet, 19)) {
    return false;
  }

  // At this point we have decoded the lengths and symbols.
  for (uint32 i = 0; i < lengthCodes + distCodes;) {
    uint32 symbol = 0;

    if (!DecodeSymbol(lengthHuffman, symbol)) {
      ZAssert(false);
      return false;
    }
    
    if (symbol < 16) {
      alphabet[i++] = static_cast<uint16>(symbol);
    }
    else {
      uint32 length = 0;

      if (symbol == 16) {
        if (i == 0) {
          ZAssert(false);
          return false;
        }

        length = alphabet[i - 1];
        symbol = 3 + ReadBits(2);
      }
      else if (symbol == 17) {
        symbol = 3 + ReadBits(3);
      }
      else {
        symbol = 11 + ReadBits(7);
      }

      if (i + symbol > lengthCodes + distCodes) {
        ZAssert(false);
        return false;
      }

      while (symbol--) {
        alphabet[i++] = (uint16)length;
      }
    }
  }

  if (alphabet[256] == 0) {
    ZAssert(false);
    return false;
  }

  // Reused lengthHuffman for decoding the original length/distance pairs.
  // They were encoded themselves with a huffman code which we needed to decode earlier.
  // Must zero out the codes before we proceed.
  memset(lengthHuffman.codes, 0, sizeof(lengthHuffman.codes));
  if (!BuildHuffman(lengthHuffman, alphabet, lengthCodes)) {
    ZAssert(false);
    return false;
  }

  // We reused the same data structure for the length codes.
  // Note that we do not need as much storage for the symbols but for simplicity we do it this way.
  PngHuffman distanceHuffman;
  memset(&distanceHuffman, 0, sizeof(PngHuffman));
  if (!BuildHuffman(distanceHuffman, alphabet + lengthCodes, distCodes)) {
    ZAssert(false);
    return false;
  }

  return DeflateStream(output, lengthHuffman, distanceHuffman);
}

bool PNG::BuildHuffman(PngHuffman& huffman, uint16* lengths, int32 count) {
  uint16 offsets[16];
  memset(offsets, 0, sizeof(offsets));

  // Read sizes in.
  for (int32 i = 0; i < count; ++i) {
    (huffman.codes[lengths[i]])++;
  }

  // Check for valid range.
  int32 left = 1;
  for (int32 i = 1; i <= MaxBits; ++i) {
    left <<= 1;
    left -= huffman.codes[i];
    if (left < 0) {
      ZAssert(false);
      return false;
    }
  }

  // Get offsets.
  offsets[1] = 0;
  for (int32 i = 1; i < MaxBits; ++i) {
    offsets[i + 1] = offsets[i] + huffman.codes[i];
  }

  // Populate symbols.
  for (int32 i = 0; i < count; ++i) {
    if (lengths[i] > 0) {
      huffman.symbols[offsets[lengths[i]]++] = static_cast<uint16>(i);
    }
  }

  return true;
}

bool PNG::DecodeSymbol(const PngHuffman& huffman, uint32& symbol) {
  int32 code = 0;
  int32 first = 0;
  int32 index = 0;

  for (uint32 i = 1; i <= MaxBits; ++i) {
    code |= static_cast<int32>(ReadBits(1));
    int32 count = huffman.codes[i];
    if (code - count < first) {
      symbol = huffman.symbols[index + (code - first)];
      return true;
    }

    index += count;
    first += count;
    first <<= 1;
    code <<= 1;
  }

  return false;
}

bool PNG::DeflateStream(uint8* output, const PngHuffman& lengthHuff, const PngHuffman& distHuff) {
  // Size base for length codes 257..285
  static const uint16 baseSizes[29] = { 
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
  // Extra bits for length codes 257..285
  static const uint16 extraBits[29] = { 
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
      3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
  // Offset base for distance codes 0..29
  static const uint16 distCodes[30] = { 
      1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
      257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
      8193, 12289, 16385, 24577 };
  // Extra bits for distance codes 0..29
  static const uint16 extraDistCodes[30] = { 
      0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
      7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
      12, 12, 13, 13 };

  // Must account for filter byte on each scan line.
  const size_t endOfImage = (mStride + 1) * mHeight;

  for (bool reading = true; reading;) {
    uint32 symbol = 0;
    if (!DecodeSymbol(lengthHuff, symbol)) {
      ZAssert(false);
      return false;
    }

    reading = (symbol != 256);
    if (symbol < 256) {
      // Literal symbol to output.
      output[mOutputIndex] = (uint8)symbol;
      mOutputIndex++;
    }
    else if (symbol > 256) {
      symbol -= 257;
      if (symbol >= 29) {
        ZAssert(false);
        return false;
      }

      uint32 numBitsToRead = extraBits[symbol];
      uint32 length = baseSizes[symbol] + ReadBits(numBitsToRead);
      if (!DecodeSymbol(distHuff, symbol)) {
        ZAssert(false);
        return false;
      }

      numBitsToRead = extraDistCodes[symbol];
      uint32 dist = distCodes[symbol] + ReadBits(numBitsToRead);

      if (dist > mOutputIndex) {
        ZAssert(false);
        return false;
      }

      while (length-- && mOutputIndex < endOfImage) {
        output[mOutputIndex] = output[mOutputIndex - dist];
        mOutputIndex++;
      }
    }
  }

  return true;
}

uint8* PNG::FilterDeflatedImage(uint8* image) {
  const size_t filteredStride = mStride + 1;

  uint8* outputImage = static_cast<uint8*>(PlatformMalloc(mStride * mHeight));
  memset(outputImage, 0, mStride * mHeight);

  for (size_t i = 0; i < mHeight; ++i) {
    const size_t rowIndex = filteredStride * i;
    const size_t rowData = rowIndex + 1;
    const size_t outputRowIndex = mStride * i;
    uint8 filterMethod = image[rowIndex];

    switch (filterMethod) {
      case PngFilter::None:
        memcpy(outputImage + outputRowIndex, image + rowData, mStride);
        break;
      case PngFilter::Sub:
      {
        for (size_t x = 0; x < mStride; ++x) {
          int64 prevIndex = x - mChannels;
          uint32 prev = (prevIndex < 0) ? 0 : outputImage[outputRowIndex + prevIndex];
          outputImage[outputRowIndex + x] = (uint8)(((uint32)image[rowData + x] + prev) % 256);
        }
      }
        break;
      case PngFilter::Up:
      {
        for (size_t x = 0; x < mStride; ++x) {
          int64 aboveIndex = (((int64)outputRowIndex) - ((int64)mStride));
          uint32 above = (aboveIndex < 0) ? 0 : outputImage[aboveIndex + x];
          outputImage[outputRowIndex + x] = (uint8)(((uint32)image[rowData + x] + above) % 256);
        }
      }
        break;
      case PngFilter::Average:
      {
        for (size_t x = 0; x < mStride; ++x) {
          int64 aboveIndex = (((int64)outputRowIndex) - ((int64)mStride));
          uint32 above = (aboveIndex < 0) ? 0 : outputImage[aboveIndex + x];

          int64 prevIndex = x - mChannels;
          uint32 prev = (prevIndex < 0) ? 0 : outputImage[outputRowIndex + prevIndex];

          outputImage[outputRowIndex + x] = (uint8)(((uint32)image[rowData + x] + ((prev + above) / 2)) % 256);
        }
      }
        break;
      case PngFilter::Paeth:
      {
        for (size_t x = 0; x < mStride; ++x) {
          int64 priorAboveIndex = (((int64)outputRowIndex) - ((int64)mStride) + ((int64)x) - ((int64)mChannels));
          uint32 priorAbove = (priorAboveIndex < 0) ? 0 : outputImage[priorAboveIndex];

          int64 aboveIndex = (((int64)outputRowIndex) - ((int64)mStride));
          uint32 above = (aboveIndex < 0) ? 0 : outputImage[aboveIndex + x];

          int64 prevIndex = x - mChannels;
          uint32 prev = (prevIndex < 0) ? 0 : outputImage[outputRowIndex + prevIndex];

          outputImage[outputRowIndex + x] = (uint8)(((int64)image[rowData + x] + PaethPredictor((int64)prev, (int64)above, (int64)priorAbove)) % 256);
        }
      }
        break;
      default:
        PlatformFree(outputImage);
        PlatformFree(image);
        ZAssert(false);
        return nullptr;
    }
  }
  
  PlatformFree(image);
  return outputImage;
}

}
