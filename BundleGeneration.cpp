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

  // TODO: Replace this with real data in the future.
  objfile.ShadingOrder().EmplaceBack(ShadingModes::UV, 2);
  objfile.AlbedoTexture() = "wall_256";

  Array<Vec3>& UVs = objfile.UVs();
  UVs.Clear();
  UVs.Resize(objfile.Verts().Size());
  for (size_t i = 0; i < UVs.Size(); ++i) {
    const float T0[] = { 0.f, 1.f };
    const float T1[] = { 0.5f, 0.f };
    const float T2[] = { 1.0f, 1.f };

    switch (i % 3) {
      case 0:
        UVs[i] = Vec3(T0[0], T0[1], 0.f);
        break;
      case 1:
        UVs[i] = Vec3(T1[0], T1[1], 0.f);
        break;
      case 2:
        UVs[i] = Vec3(T2[0], T2[1], 0.f);
        break;
      default:
        break;
    }
  }

  MemorySerializer objSerializer;
  objloader.Serialize(objSerializer);

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

void SerializeTexture(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory) {
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

}
