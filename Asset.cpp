#include "Asset.h"

namespace ZSharp {
Asset::Asset()
  : mSize(0), mLoose(false), mAssetType(AssetType::Unbound) {
}

Asset::Asset(size_t size, const String& name, const String& extension, bool loose, const FileString& loosePath, AssetType type)
  : mSize(size), mName(name), mExtension(extension), mLoose(loose), mLoosePath(loosePath), mAssetType(type) {
}

Asset::Asset(const Asset& rhs)
  : mSize(rhs.mSize), 
  mDeserializer(rhs.mDeserializer),
  mName(rhs.mName), 
  mExtension(rhs.mExtension), 
  mLoose(rhs.mLoose), 
  mLoosePath(rhs.mLoosePath), 
  mAssetType(rhs.mAssetType) {
}

const String& Asset::Name() const {
  return mName;
}

const String& Asset::Extension() const {
  return mExtension;
}

bool Asset::IsLoose() const {
  return mLoose;
}

const FileString& Asset::LoosePath() const {
  return mLoosePath;
}

void Asset::SetLoaderOffset(void* memory, size_t size) {
  if (mDeserializer.BaseAddress() == nullptr) {
    mDeserializer.Reassign(memory);
    mSize = size;
  }
}

const MemoryDeserializer& Asset::Loader() const {
  return mDeserializer;
}

AssetType Asset::Type() const {
  return mAssetType;
}

void Asset::Serialize(ISerializer& serializer) {
  serializer.Serialize(&mSize, sizeof(mSize));
  mName.Serialize(serializer);
  mExtension.Serialize(serializer);
  serializer.Serialize(&mLoose, sizeof(mLoose));
  mLoosePath.GetAbsolutePath().Serialize(serializer);
  serializer.Serialize(&mAssetType, sizeof(mAssetType));
}

void Asset::Deserialize(IDeserializer& deserializer) {
  deserializer.Deserialize(&mSize, sizeof(mSize));
  mName.Deserialize(deserializer);
  mExtension.Deserialize(deserializer);
  deserializer.Deserialize(&mLoose, sizeof(mLoose));
  String loosePath;
  loosePath.Deserialize(deserializer);
  mLoosePath = loosePath;
  deserializer.Deserialize(&mAssetType, sizeof(mAssetType));
}

}
