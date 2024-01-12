#include "BundleGeneration.h"

#include "OBJFile.h"
#include "PNG.h"
#include "JPEG.h"
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
  objfile.LoadFromFile(filename);

  if (!objfile.AlbedoTexture().IsEmpty()) {
    objfile.ShadingOrder().EmplaceBack(ShadingModes::UV, 2);
  }

  MemorySerializer objSerializer;
  objfile.Serialize(objSerializer);

  bundleAssets.EmplaceBack(objSerializer.Size(),
    filename.GetFilename(),
    filename.GetExtension(),
    false,
    FileString(""),
    AssetType::Model);
  const size_t objSerializerSize = objSerializer.Size();
  bundleMemory.Serialize(&objSerializerSize, sizeof(objSerializerSize));
  bundleMemory.Serialize(objSerializer.Data(), objSerializerSize);
}

void SerializeTexturePNG(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory) {
  PNG png(filename);
  MemorySerializer pngSerializer;
  png.Serialize(pngSerializer);

  bundleAssets.EmplaceBack(pngSerializer.Size(),
    filename.GetFilename(),
    filename.GetExtension(),
    false,
    FileString(""),
    AssetType::Texture);

  const size_t pngSerializerSize = pngSerializer.Size();
  bundleMemory.Serialize(&pngSerializerSize, sizeof(pngSerializerSize));
  bundleMemory.Serialize(pngSerializer.Data(), pngSerializerSize);
}

void SerializeTextureJPG(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory) {
  JPEG jpg(filename);
  MemorySerializer pngSerializer;
  jpg.Serialize(pngSerializer);

  bundleAssets.EmplaceBack(pngSerializer.Size(),
    filename.GetFilename(),
    filename.GetExtension(),
    false,
    FileString(""),
    AssetType::Texture);

  const size_t pngSerializerSize = pngSerializer.Size();
  bundleMemory.Serialize(&pngSerializerSize, sizeof(pngSerializerSize));
  bundleMemory.Serialize(pngSerializer.Data(), pngSerializerSize);
}

}
