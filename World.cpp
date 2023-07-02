#include "World.h"

#include "Bundle.h"
#include "OBJFile.h"
#include "OBJLoader.h"
#include "PNG.h"

#include "CommonMath.h"

#define DEBUG_TEXTURE 1

namespace ZSharp {
World::World() {
}

void World::LoadModels() {
  {
    Model model;
    mActiveModels.PushBack(model);
  }

  {
    VertexBuffer vertBuffer;
    mVertexBuffers.PushBack(vertBuffer);
  }

  {
    IndexBuffer indexBuffer;
    mIndexBuffers.PushBack(indexBuffer);
  }
  
  Model& cachedModel = mActiveModels[mActiveModels.Size() - 1];
  LoadOBJ(cachedModel);

  VertexBuffer& cachedVertBuffer = mVertexBuffers[mVertexBuffers.Size() - 1];
  IndexBuffer& cachedIndexBuffer = mIndexBuffers[mIndexBuffers.Size() - 1];

  size_t indexBufSize = 0;
  size_t vertBufSize = 0;
  size_t vertStride = 0;
  for (Mesh& mesh : cachedModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertBufSize += mesh.GetVertTable().Size();
    vertStride = mesh.Stride();
  }

  cachedIndexBuffer.Resize(indexBufSize);
  cachedVertBuffer.Resize(vertBufSize, vertStride);
}

void World::DebugLoadTriangle(const float* v1, const float* v2, const float* v3, ShadingModeOrder order, size_t stride)
{
  {
    Model model(order, stride);
    mActiveModels.PushBack(model);
  }

  {
    VertexBuffer vertBuffer;
    mVertexBuffers.PushBack(vertBuffer);
  }

  {
    IndexBuffer indexBuffer;
    mIndexBuffers.PushBack(indexBuffer);
  }

  Model& cachedModel = mActiveModels[mActiveModels.Size() - 1];
  cachedModel.CreateNewMesh();

  Bundle& bundle = Bundle::Get();
  if (bundle.Assets().Size() == 0) {
    return;
  }

  Asset* pngAsset = bundle.GetAsset("wall_256");
  
  if (pngAsset == nullptr) {
    ZAssert(false);
    return;
  }

  MemoryDeserializer pngDeserializer(pngAsset->Loader());

  PNG texture;
  texture.Deserialize(pngDeserializer);
  uint8* pngData = texture.Decompress(ChannelOrder::BGR);
  size_t width = texture.GetWidth();
  size_t height = texture.GetHeight();
  size_t channels = texture.GetNumChannels();
  cachedModel.BindTexture(pngData, width, height, channels);

  {
    const size_t strideBytes = stride * sizeof(float);
    Mesh& firstMesh = cachedModel[0];
    size_t vertSize = 3 * stride;
    size_t faceSize = 1;
    firstMesh.Resize(vertSize, faceSize);
    firstMesh.SetData(v1, 0, strideBytes);
    firstMesh.SetData(v2, stride, strideBytes);
    firstMesh.SetData(v3, stride * 2, strideBytes);
    Triangle triangle(0, 1, 2);
    firstMesh.SetTriangle(triangle, 0);
  }

  VertexBuffer& cachedVertBuffer = mVertexBuffers[mVertexBuffers.Size() - 1];
  IndexBuffer& cachedIndexBuffer = mIndexBuffers[mIndexBuffers.Size() - 1];

  size_t indexBufSize = 0;
  size_t vertBufSize = 0;
  for (Mesh& mesh : cachedModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertBufSize += mesh.GetVertTable().Size();
  }

  cachedIndexBuffer.Resize(indexBufSize);
  cachedVertBuffer.Resize(vertBufSize, stride);
}

size_t World::GetTotalModels() const {
  return mActiveModels.Size();
}

Array<Model>& World::GetModels() {
  return mActiveModels;
}

Array<VertexBuffer>& World::GetVertexBuffers() {
  return mVertexBuffers;
}

Array<IndexBuffer>& World::GetIndexBuffers() {
  return mIndexBuffers;
}

void World::LoadOBJ(Model& model) {
  Bundle& bundle = Bundle::Get();
  if (bundle.Assets().Size() == 0) {
    return;
  }

  const Asset& objAsset = bundle.Assets()[0];
  MemoryDeserializer objDeserializer(objAsset.Loader());

  OBJFile objFile;
  OBJLoader objLoader(objDeserializer, objFile);

  model.CreateNewMesh();
  model.SetShadingOrder(objFile.ShadingOrder());
  model.SetStride(objFile.Stride());
  Mesh& mesh = model[0];

  /*
  X, Y, Z, W
  R, G, B
  */
#if DEBUG_TEXTURE
  const size_t stride = 4 + 2; // TODO: Make this based off of the source asset.

  Asset* pngAsset = bundle.GetAsset("wall_256");
  
  if (pngAsset == nullptr) {
    ZAssert(false);
    return;
  }

  MemoryDeserializer pngDeserializer(pngAsset->Loader());

  PNG texture;
  texture.Deserialize(pngDeserializer);
  uint8* pngData = texture.Decompress(ChannelOrder::BGR);
  size_t width = texture.GetWidth();
  size_t height = texture.GetHeight();
  size_t channels = texture.GetNumChannels();
  model.BindTexture(pngData, width, height, channels);

#else
  const size_t stride = 4 + 3; // TODO: Make this based off of the source asset.
#endif

  size_t vertSize = objFile.Verts().Size() * stride;
  size_t indexSize = objFile.Faces().Size();

  mesh.Resize(vertSize, indexSize);

  for (size_t i = 0, triIndex = 0; i < objFile.Verts().Size(); ++i) {
    const Vec4& vector = objFile.Verts()[i];
    ZAssert(FloatEqual(vector[3], 1.f));

    float vertex[stride];
    memcpy(vertex, reinterpret_cast<const float*>(&vector), sizeof(Vec4));

#if DEBUG_TEXTURE
    const float T0[] = { 0.f, 1.f };
    const float T1[] = { 0.5f, 0.f };
    const float T2[] = { 1.0f, 1.f };

    switch (triIndex % 3) {
      case 0:
        memcpy(vertex + 4, T0, sizeof(T0));
        break;
      case 1:
        memcpy(vertex + 4, T1, sizeof(T1));
        break;
      case 2:
        memcpy(vertex + 4, T2, sizeof(T2));
        break;
      default:
        break;
    }
#else
    const float R[] = { 1.f, 0.f, 0.f };
    const float G[] = { 0.f, 1.f, 0.f };
    const float B[] = { 0.f, 0.f, 1.f };

    switch (triIndex % 3) {
      case 0:
        memcpy(vertex + 4, R, sizeof(R));
        break;
      case 1:
        memcpy(vertex + 4, G, sizeof(G));
        break;
      case 2:
        memcpy(vertex + 4, B, sizeof(B));
        break;
      default:
        break;
    }
#endif

    triIndex++;

    mesh.SetData(vertex, i * stride, sizeof(vertex));
  }

  const Array<OBJFace>& faceList = objFile.Faces();
  for (size_t triIndex = 0; triIndex < indexSize; ++triIndex) {
    Triangle triangle(static_cast<size_t>(faceList[triIndex].triangleFace[0].vertexIndex),
      static_cast<size_t>(faceList[triIndex].triangleFace[1].vertexIndex),
      static_cast<size_t>(faceList[triIndex].triangleFace[2].vertexIndex)
    );
    mesh.SetTriangle(triangle, triIndex);
  }
}
}
