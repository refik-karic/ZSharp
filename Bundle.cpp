#include "Bundle.h"

#include "ZConfig.h"

const size_t BundleVersion = 0;

namespace ZSharp {

bool GenerateBundle(const FileString& filename, Array<Asset>& assets, MemorySerializer& data) {
  FileSerializer fileSerializer(filename);
  if (!fileSerializer.Serialize(&BundleVersion, sizeof(BundleVersion))) {
    return false;
  }

  assets.Serialize(fileSerializer);

  if (!fileSerializer.Serialize(data.Data(), data.Size())) {
    return false;
  }

  return true;
}

Bundle::Bundle(const FileString& filename) : mHandle(filename) {
  if (mHandle.IsOpen()) {
    MemoryDeserializer deserializer(mHandle.GetBuffer());
    Deserialize(deserializer);
  }
}

Bundle& Bundle::Get() {
  ZConfig& config = ZConfig::GetInstance();
  static Bundle bundle(config.GetAssetPath());
  return bundle;
}

Asset* Bundle::GetAsset(const String& name) {
  Asset* foundAsset = nullptr;

  for (Asset& asset : mAssets) {
    if (asset.Name() == name) {
      foundAsset = &asset;
      break;
    }
  }

  return foundAsset;
}

const Array<Asset>& Bundle::Assets() const {
  return mAssets;
}

bool Bundle::Deserialize(MemoryDeserializer& deserializer) {
  /*
    1) Read any header info
      1a) Bundle Version
      1b) Serialized Asset objects
      1c) Read serialized block size
    2) Read a block of serialized memory
      2a) Size means how many bytes the next serialized blob is
      2b) Followed by Size bytes of serialized asset data
      2c) Repeat for N assets
  */
  
  size_t bundleVersion = 0;
  if (!deserializer.Deserialize(&bundleVersion, sizeof(bundleVersion))) {
    return false;
  }

  if (bundleVersion != BundleVersion) {
    return false;
  }

  mAssets.Deserialize(deserializer);

  // TODO: Add this to the serializer so that we don't make assumptions.
  const size_t padding = sizeof(size_t);

  // Skip over memory block size.
  deserializer.Reassign(deserializer.BaseAddress() + deserializer.Offset() + padding);

  for (size_t i = 0; i < mAssets.Size(); ++i) {
    size_t serializedSize = 0;
    if (!deserializer.Deserialize(&serializedSize, sizeof(serializedSize))) {
      return false;
    }

    Asset& asset = mAssets[i];
    uint8* offset = deserializer.BaseAddress() + deserializer.Offset() + padding;

    // Set the asset load point and skip ahead to the next block of memory.
    asset.SetLoaderOffset(offset, serializedSize);
    deserializer.Reassign(offset + serializedSize);
  }

  return true;
}

}

