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

PNG::PNG(const FileString& filename)
  : mFilename(filename), mReader(filename) {
  ReadHeader();
}

PNG::~PNG() {
}

bool PNG::Decompress(uint8* buffer) {
  if (!mReader.IsOpen()) {
    return false;
  }

  ZAssert(buffer == nullptr);

  uint32 length = 0;
  uint32 type = 0;

  IdentifyChunk(length, type);

  // At this point we assume that we have seeked to the proper IDAT chunk.
  if (memcmp(&type, "IDAT", 4)) {
    ZAssert(false);
    return false;
  }

  buffer = (uint8*)PlatformMalloc(mStride * mHeight);
  size_t outputBufferPos = 0;

  // Sanity check for zlib header DEFLATE compression method.
  if ((DataOffset()[0] & 0x0F) != 8) {
    ZAssert(false);
    return false;
  }

  // Skip over first two zlib header bytes.
  // PNG spec does not allow the optional preset dictionary so we skip directly to the DEFLATE stream.
  mDataOffset += 2;

  for (bool reading = true; reading;) {
    uint32 isFinal = ReadBits(1);
    reading = (isFinal == 0);

    uint32 compressedType = ReadBits(2);
    if (compressedType == 0) {
      mBitOffset = RoundUpNearestMultiple(mBitOffset, 8);
      
      // TODO: This is a "stored"/uncompressed block.
      //  2 next bytes are LEN
      //  2 next bytes are NLEN (ones-complement of previous length)
      //  Next LEN bytes are uncompressed data that should be copied to the output.

      const uint8* data = DataBitOffset();

      uint16 uncompressedLength = data[0];
      uncompressedLength |= (((uint16)(data[1])) << 8);
      uint16 onesCompLength = data[2];
      onesCompLength |= (((uint16)(data[3])) << 8);

      if ((uncompressedLength) != (~onesCompLength)) {
        ZAssert(false);
        return false;
      }

      mBitOffset += 32;
      mBitOffset += (uncompressedLength * 8);

      memcpy(buffer, data + 4, uncompressedLength);
      outputBufferPos += uncompressedLength;
    }
    else if (compressedType == 2) {
      DecodeDynamicHuffman(buffer);
    }
  }

  return true;
}

bool PNG::ReadHeader() {
  if (!mReader.IsOpen()) {
    ZAssert(false);
    return false;
  }

  const size_t MagicHeaderLength = 8;
  const size_t CRCLength = 4;

  const uint8* data = reinterpret_cast<const uint8*>(mReader.GetBuffer());

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
    mBitsPerPixel = bitDepth;
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
    mBitsPerPixel = bitDepth;
    mChannels = 4;
  }
  else {
    // Implement other formats as necessary.
    ZAssert(false);
    return false;
  }

  const size_t fileSize = mReader.GetSize();

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

    if (mDataOffset >= fileSize) {
      ZAssert(false);
      return false;
    }
  }

  return true;
}

const uint8* PNG::DataOffset() {
  return reinterpret_cast<const uint8*>(mReader.GetBuffer()) + mDataOffset;
}

const uint8* PNG::DataBitOffset() {
  return reinterpret_cast<const uint8*>(mReader.GetBuffer()) + mDataOffset + (mBitOffset / 8);
}

uint32 PNG::ReadBits(size_t count) {
  uint32 ret = 0;

  if (count > MaxBits || count == 0) {
    ZAssert(false);
    return 0;
  }

  // TODO: Come up with a faster implementation for reading bits.
  //  We do some funky math here to get the iteration count.
  const uint8* data = DataBitOffset();
  const size_t bitsReadInByte = (mBitOffset % 8);
  const size_t numIterations = ((8 - bitsReadInByte) >= count) ? 1 : ((RoundUpNearestMultiple(count, 8) / 8) + 1);

  for (size_t i = 0; i < numIterations; ++i) {
    if (i > 0) {
      ret <<= 8;
    }

    ret |= *data;
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

void PNG::DecodeDynamicHuffman(uint8* output) {
  /*
  Steps:
    1) Decode both Huffman Trees for literals and lengths.
    2) Build the Huffman Trees using the decoded data.
    3) Walk the stream and decode symbols using the decoded Huffman Trees.
    4) Search for any LZ77 length/distance pairs and write them to the output.
  */
  (void)output;

  uint32 lengthCodes = ReadBits(5) + 257;
  uint32 distCodes = ReadBits(5) + 1;
  uint32 codeCodes = ReadBits(4) + 4;

  if (lengthCodes > 286 || distCodes > 32 || codeCodes > 19) {
    ZAssert(false);
    return;
  }

  // TODO: Finish this up, correct so far...
  uint8 alphabet[19];
  memset(alphabet, 0, sizeof(alphabet));
  const uint8 alphabetOrder[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

  for (uint32 i = 0; i < codeCodes; ++i) {
    uint32 symbol = ReadBits(3);
    alphabet[alphabetOrder[i]] = static_cast<uint8>(symbol);
  }

  PngHuffman huffman;
  memset(&huffman, 0, sizeof(huffman));
  BuildHuffman(huffman, alphabet, 19);
}

void PNG::BuildHuffman(PngHuffman& huffman, uint8* lengths, int32 count) {
  uint8 sizes[16];
  memset(sizes, 0, sizeof(sizes));

  // Read sizes in.
  for (int32 i = 0; i < count; ++i) {
    sizes[lengths[i]]++;
  }

  // Check for valid range.
  for (int32 i = 1; i < 16; ++i) {
    if (sizes[i] > (1 << i)) {
      ZAssert(false);
      return;
    }
  }

  // TODO: Finish...
  int32 code = 0;
  for (int32 i = 0; i < 16; ++i) {
    huffman.lengthCodes[i] = static_cast<uint16>(code);

  }

  (void)huffman;
}

}
