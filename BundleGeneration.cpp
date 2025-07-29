#include "BundleGeneration.h"

#include "ZAssert.h"
#include "OBJFile.h"
#include "Model.h"
#include "Mesh.h"
#include "PNG.h"
#include "JPEG.h"
#include "Bundle.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"
#include "CommonMath.h"

namespace ZSharp {

bool GenerateBundle(const FileString& filename, Array<Asset>& assets, MemorySerializer& data) {
  FileSerializer fileSerializer(filename);
  if (!fileSerializer.Serialize(&BundleVersion, sizeof(BundleVersion))) {
    return false;
  }

  size_t numAssets = assets.Size();
  fileSerializer.Serialize(&numAssets, sizeof(numAssets));
  for (Asset& asset : assets) {
    asset.Serialize(fileSerializer);
  }

  if (!fileSerializer.Serialize(data.Data(), data.Size())) {
    return false;
  }

  return true;
}

void SerializeOBJFile(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory) {
  // Open and parse the OBJ file.
  // Pack data into runtime format for mesh rendering.
  // Serialize all necessary fields to a block of memory.
  // Save an asset object in the header of the bundle.
  // Append this serialized block of memory to main bundle memory block.
  OBJFile objfile;
  objfile.LoadFromFile(filename);

  Model model;
  Mesh& mesh = model.GetMesh();

  const int32 numVerts = (int32)objfile.Verts().Size();
  const float* vertData = (const float*)objfile.Verts().GetData();

  // NOTE: We calculate the AABB on the packed vert data from the OBJ file.
  //  This lets us use a wider optimized version of the algorithm.
  model.BoundingBox() = ComputeBoundingBox(4, vertData, numVerts * 4);

  if (!objfile.AlbedoTexture().IsEmpty()) {
    ShaderDefinition shader(4, 4, ShadingMethod::UV);
    mesh.SetShader(shader);

    // NOTE: Stride changes depending on bound shader, we must check after applying.
    const size_t stride = mesh.Stride();

    mesh.Resize(objfile.Verts().Size() * stride, objfile.Faces().Size());
    const float* uvData = (const float*)objfile.UVs().GetData();

    for (int32 i = 0; i < numVerts; ++i) {
      ZAssert(vertData[3] == 1.f);

      const size_t index = i * stride;

      mesh.SetData(vertData, index, 4 * sizeof(float));
      mesh.SetData(uvData, index + 4, 2 * sizeof(float));

      vertData += 4;
      uvData += 3;
    }
  }
  else {
    ShaderDefinition shader(4, 4, ShadingMethod::RGB);
    mesh.SetShader(shader);

    // NOTE: Stride changes depending on bound shader, we must check after applying.
    const size_t stride = mesh.Stride();

    mesh.Resize(objfile.Verts().Size() * stride, objfile.Faces().Size());

    const float R[] = { 1.f, 0.f, 0.f };
    const float G[] = { 0.f, 1.f, 0.f };
    const float B[] = { 0.f, 0.f, 1.f };

    for (int32 i = 0, triIndex = 0; i < numVerts; ++i) {
      ZAssert(vertData[3] == 1.f);

      const size_t index = i * stride;
      mesh.SetData(vertData, index, 4 * sizeof(float));

      switch (triIndex) {
        case 0:
        {
          mesh.SetData(R, index + 4, sizeof(R));
        }
        break;
        case 1:
        {
          mesh.SetData(G, index + 4, sizeof(G));
        }
        break;
        case 2:
        {
          mesh.SetData(B, index + 4, sizeof(B));
        }
        break;
        default:
          break;
      }

      vertData += 4;
      triIndex = (triIndex == 2) ? 0 : ++triIndex;
    }
  }

  const int32 stride = (int32)mesh.Stride();
  int32 indexSize = (int32)objfile.Faces().Size();

  const Array<OBJFace>& faceList = objfile.Faces();
  for (int32 triIndex = 0; triIndex < indexSize; ++triIndex) {
    Triangle triangle(static_cast<int32>(faceList[triIndex].triangleFace[0].vertexIndex) * stride,
      static_cast<int32>(faceList[triIndex].triangleFace[1].vertexIndex) * stride,
      static_cast<int32>(faceList[triIndex].triangleFace[2].vertexIndex) * stride
    );
    mesh.SetTriangle(triangle, triIndex);
  }

  // TODO: Avoid duplicating this state at some point.
  //  The OBJ to TexturePool mapping isn't set up until the texture is loaded at runtime.
  //  We could make the order more deterministic during bundling.
  mesh.AlbedoTexture() = objfile.AlbedoTexture();

  MemorySerializer modelSerializer;
  model.Serialize(modelSerializer);

  bundleAssets.EmplaceBack(modelSerializer.Size(),
    filename.GetFilename(),
    filename.GetExtension(),
    false,
    FileString(""),
    AssetType::Model);
  const size_t modelSerializerSize = modelSerializer.Size();
  bundleMemory.Serialize(&modelSerializerSize, sizeof(modelSerializerSize));
  bundleMemory.Serialize(modelSerializer.Data(), modelSerializerSize);
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
