#pragma once

#include "ZBaseTypes.h"
#include "UIBase.h"
#include "Texture.h"
#include "ZString.h"

namespace ZSharp {

class UIImage : public UIBase {
  public:

  UIImage(size_t width, size_t height, const String& name);

  ~UIImage();

  void SetTextureId(int32 id);

  Texture* GetTexture();

  private:
  int32 mTextureId = -1;
};

}
