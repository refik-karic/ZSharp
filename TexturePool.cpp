#include "TexturePool.h"

#include "PNG.h"
#include "JPEG.h"

#include "ScopedTimer.h"

namespace ZSharp {

TexturePool& TexturePool::Get() {
  static TexturePool pool;
  return pool;
}

TexturePool::TexturePool() {
}

TexturePool::~TexturePool() {
}

int32 TexturePool::LoadTexture(Asset& asset) {
  if (asset.Type() != AssetType::Texture) {
    return -1;
  }

  const String& assetName = asset.Name();

  if (mLoadedTextures.HasKey(assetName)) {
    return mLoadedTextures[assetName];
  }

  if (asset.Extension() == "png") {
    MemoryDeserializer pngDeserializer(asset.Loader());

    PNG png;
    png.Deserialize(pngDeserializer);
    uint8* pngData = png.Decompress(ChannelOrderPNG::BGR);
    size_t width = png.GetWidth();
    size_t height = png.GetHeight();
    size_t channels = png.GetNumChannels();

    Texture& texture = mTextures.EmplaceBack();
    texture.Assign(pngData, channels, width, height);
    texture.GenerateMips();

    int32 index = ((int32)mTextures.Size()) - 1;
    mLoadedTextures.Add(assetName, index);
    return index;
  }
  else if (asset.Extension() == "jpg") {
    MemoryDeserializer jpgDeserializer(asset.Loader());

    JPEG jpg;
    jpg.Deserialize(jpgDeserializer);
    uint8* jpgData = jpg.Decompress(ChannelOrderJPG::BGR);
    size_t width = jpg.GetWidth();
    size_t height = jpg.GetHeight();
    size_t channels = jpg.GetNumChannels();

    if (channels == 3) {
      jpgData = InsertAlphaChannel(jpgData, width, height);
      ++channels;
    }

    Texture& texture = mTextures.EmplaceBack();
    texture.Assign(jpgData, channels, width, height);
    texture.GenerateMips();

    int32 index = ((int32)mTextures.Size()) - 1;
    mLoadedTextures.Add(assetName, index);
    return index;
  }
  else {
    return -1;
  }
}

Texture* TexturePool::GetTexture(int32 id) {
  if (id < 0) {
    return nullptr;
  }
  else {
    return &mTextures[id];
  }
}

}
