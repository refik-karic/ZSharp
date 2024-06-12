#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZString.h"
#include "Serializer.h"
#include "ISerializable.h"

namespace ZSharp {

enum class AssetType {
  Unbound,
  Model,
  Texture,
  Audio
};

class Asset final : public ISerializable {
  public:
  Asset();

  // Used during bundling.
  Asset(size_t size, 
    const String& name,
    const String& extension,
    bool loose,
    const FileString& loosePath,
    AssetType type);

  Asset(const Asset& rhs);

  const String& Name() const;

  const String& Extension() const;

  bool IsLoose() const;

  const FileString& LoosePath() const;

  void SetLoaderOffset(void* memory, size_t size);

  const MemoryDeserializer& Loader() const;

  AssetType Type() const;

  virtual void Serialize(ISerializer& serializer) override;

  virtual void Deserialize(IDeserializer& deserializer) override;

  private:
  size_t mSize;
  MemoryDeserializer mDeserializer;
  String mName;
  String mExtension;
  bool mLoose;
  FileString mLoosePath;
  AssetType mAssetType;
};

}