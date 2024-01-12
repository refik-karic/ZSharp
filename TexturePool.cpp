#include "TexturePool.h"

#include "PNG.h"

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

  MemoryDeserializer pngDeserializer(asset.Loader());

  PNG png;
  png.Deserialize(pngDeserializer);
  uint8* pngData = png.Decompress(ChannelOrderPNG::BGR);
  size_t width = png.GetWidth();
  size_t height = png.GetHeight();
  size_t channels = png.GetNumChannels();

  Texture& texture = mTextures.EmplaceBack();
  texture.Assign(pngData, channels, width, height);

  int32 index = ((int32)mTextures.Size()) - 1;
  mLoadedTextures.Add(assetName, index);
  return index;
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
