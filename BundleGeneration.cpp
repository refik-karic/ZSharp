#include "BundleGeneration.h"

#include "OBJFile.h"
#include "OBJLoader.h"
#include "PNG.h"
#include "Bundle.h"

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

void SerializeOBJFile(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory) {
  // Open and parse the OBJ file.
  // Serialize all necessary fields to a block of memory.
  // Save an asset object in the header of the bundle.
  // Append this serialized block of memory to main bundle memory block.
  OBJFile objfile;
  OBJLoader objloader(filename, objfile);
  MemorySerializer objSerializer;
  objloader.Serialize(objSerializer);

  Asset bundledAsset(objSerializer.Size(),
    filename.GetFilename(),
    filename.GetExtension(),
    false,
    FileString(""),
    AssetType::Model);

  bundleAssets.PushBack(bundledAsset);
  const size_t objSerializerSize = objSerializer.Size();
  bundleMemory.Serialize(&objSerializerSize, sizeof(objSerializerSize));
  bundleMemory.Serialize(objSerializer.Data(), objSerializerSize);
}

void SerializeTexture(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory) {
  PNG png(filename);
  MemorySerializer pngSerializer;
  png.Serialize(pngSerializer);

  Asset bundledAsset(pngSerializer.Size(),
    filename.GetFilename(),
    filename.GetExtension(),
    false,
    FileString(""),
    AssetType::Texture);

  bundleAssets.PushBack(bundledAsset);

  const size_t pngSerializerSize = pngSerializer.Size();
  bundleMemory.Serialize(&pngSerializerSize, sizeof(pngSerializerSize));
  bundleMemory.Serialize(pngSerializer.Data(), pngSerializerSize);
}

}
