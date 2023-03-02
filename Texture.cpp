#include "Texture.h"

namespace ZSharp {

Texture::Texture(uint8* data,
  size_t bitsPerPixel, 
  size_t numChannels, 
  size_t totalSize, 
  size_t width, 
  size_t height) 
{
  (void)data;
  (void)bitsPerPixel;
  (void)numChannels;
  (void)totalSize;
  (void)width;
  (void)height;
}

Texture::~Texture() {
}

}
