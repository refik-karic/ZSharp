#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Asset.h"
#include "Texture.h"
#include "HashTable.h"

namespace ZSharp {

class TexturePool final {
  public:

  TexturePool();

  ~TexturePool();

  int32 LoadTexture(Asset& asset);

  Texture* GetTexture(int32 id);

  private:
  HashTable<String, int32> mLoadedTextures;
  Array<Texture> mTextures;
};

extern TexturePool* GlobalTexturePool;

}
