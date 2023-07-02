#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZString.h"
#include "Asset.h"
#include "Array.h"
#include "Serializer.h"
#include "ZFile.h"

namespace ZSharp {

static const size_t BundleVersion;

/*
A bundle contains a collection of assets.
These can be loose or serialized assets.
Loose assets are not stored in the bundle itself, they are just placeholders.
Order matters. Assets are stored sequentially in the serialized section as they appear in the header.
*/
class Bundle final {
  public:

  Bundle(const Bundle&) = delete;
  void operator=(const Bundle&) = delete;

  static Bundle& Get();

  Asset* GetAsset(const String& name);

  const Array<Asset>& Assets() const;

  private:
  Array<Asset> mAssets;
  MemoryMappedFileReader mHandle;

  Bundle(const FileString& filename);

  bool Deserialize(MemoryDeserializer& deserializer);
};

}
