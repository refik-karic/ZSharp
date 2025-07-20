#include "UIImage.h"

#include "TexturePool.h"

namespace ZSharp {

UIImage::UIImage(size_t width, size_t height, const String& name)
  : UIBase(width, height, name) {
}

UIImage::~UIImage() {
}

void UIImage::SetTextureId(int32 id) {
  mTextureId = id;
}

Texture* UIImage::GetTexture() {
  if (mTextureId != -1) {
    return GlobalTexturePool->GetTexture(mTextureId);
  }
  else {
    return nullptr;
  }
}

}
