#include "World.h"

#include "Bundle.h"
#include "CommonMath.h"
#include "Logger.h"
#include "OBJFile.h"
#include "PhysicsAlgorithms.h"
#include "ScopedTimer.h"
#include "TexturePool.h"

#include <cstring>

#define DEBUG_FLAT_SHADE_RGB 0
#define PHYSICS_ENABLE 1
#define PHYSICS_DISABLE_FORCES 0

namespace ZSharp {
World::World() {
}

void World::TickPhysics(size_t deltaMs) {
  NamedScopedTimer(WorldTickPhysics);

  Logger::Log(LogCategory::Perf, String::FromFormat("Ticking physics simulation for {0}ms.\n", deltaMs));

#if !PHYSICS_ENABLE
  return;
#endif

  // Update forces for all dynamic objects at the start of the time step.
  for (PhysicsObject*& currentObject : mDynamicObjects) {
    // Accumulate current forces for this frame.
    currentObject->Velocity() += Vec3(0.f, GravityPerSecond * (float)deltaMs, 0.f);
  }

  for (PhysicsObject*& currentObject : mDynamicObjects) {
    for (PhysicsObject*& staticObject : mStaticObjects) {
      // Check for collision against static objects.
      float t0 = 0.f;
      float t1 = 1.f;
      float timeOfImpact = 0.f;

      if (StaticContinuousTest(*currentObject, *staticObject, t0, t1, timeOfImpact)) {
        //currentObject->OnCollisionStartDelegate(staticObject);
        CorrectOverlappingObjects(*currentObject, *staticObject);
        //currentObject->OnCollisionEndDelegate(staticObject);
      }
      else if (currentObject->TransformedAABB().Intersects(staticObject->TransformedAABB())) {
        CorrectOverlappingObjects(*currentObject, *staticObject);
      }
    }

#if 0
    for (PhysicsObject*& dynamicObject : mDynamicObjects) {
      // Check for collision against dynamic objects.
      if (currentObject == dynamicObject) {
        continue;
      }


    }
#endif
  }

#if !PHYSICS_DISABLE_FORCES
  // Update positions for the current timestep.
  for (PhysicsObject*& currentObject : mDynamicObjects) {
    currentObject->Position() += currentObject->Velocity();
  }
#endif
}

void World::LoadModels() {
  Bundle& bundle = Bundle::Get();
  if (bundle.Assets().Size() == 0) {
    return;
  }

  for (Asset& asset : bundle.Assets()) {
    if (asset.Type() == AssetType::Model) {
      Model& model = mActiveModels.EmplaceBack();
      VertexBuffer& vertBuffer = mVertexBuffers.EmplaceBack();
      IndexBuffer& indexBuffer = mIndexBuffers.EmplaceBack();

      LoadOBJ(model, asset);

      /*
        TODO: Move this stuff to the bundle generation phase.
          We're only setting them on load right now because we're still trying to figure out how everything will work.
      */
      model.BoundingBox() = model.ComputeBoundingBox();

      // TODO: Remove me.
      if (asset.Name() == "plane") {
        model.Tag() = PhysicsTag::Static;
      }
      else {
#if PHYSICS_ENABLE
        model.Position() += Vec3(0.f, 15.f, 0.f);
#endif
        model.Tag() = PhysicsTag::Dynamic;
      }

      size_t indexBufSize = 0;
      size_t vertBufSize = 0;
      size_t vertStride = 0;
      for (Mesh& mesh : model.GetMeshData()) {
        indexBufSize += (mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
        vertBufSize += mesh.GetVertTable().Size();
        vertStride = mesh.Stride();
      }

      indexBuffer.Resize(indexBufSize);
      vertBuffer.Resize(vertBufSize, vertStride);

      switch (model.Tag()) {
        case PhysicsTag::Dynamic:
          mDynamicObjects.PushBack(&model);
          break;
        case PhysicsTag::Static:
          mStaticObjects.PushBack(&model);
          break;
        case PhysicsTag::Unbound:
          Logger::Log(LogCategory::Info, String::FromFormat("Asset {0} will not be considered for physics.\n", asset.Name()));
          break;
      }
    }
  }
}

void World::DebugLoadTriangle(const float* v1, const float* v2, const float* v3, ShadingModeOrder order, size_t stride)
{
  Model& model = mActiveModels.EmplaceBack(order, stride);
  VertexBuffer& vertBuffer = mVertexBuffers.EmplaceBack();
  IndexBuffer& indexBuffer = mIndexBuffers.EmplaceBack();

  model.CreateNewMesh();

  const bool isTextureMapped = order.Contains(ShadingMode(ShadingModes::UV, 2));

  if (isTextureMapped) {
    Bundle& bundle = Bundle::Get();
    if (bundle.Assets().Size() == 0) {
      return;
    }

    Asset* textureAsset = bundle.GetAsset("wall_256");

    if (textureAsset == nullptr) {
      ZAssert(false);
      return;
    }

    model.TextureId() = TexturePool::Get().LoadTexture(*textureAsset);
  }

  {
    const size_t strideBytes = stride * sizeof(float);
    Mesh& firstMesh = model[0];
    size_t vertSize = 3 * stride;
    size_t faceSize = 1;
    firstMesh.Resize(vertSize, faceSize);
    firstMesh.SetData(v1, 0, strideBytes);
    firstMesh.SetData(v2, stride, strideBytes);
    firstMesh.SetData(v3, stride * 2, strideBytes);
    Triangle triangle(0, 1, 2);
    firstMesh.SetTriangle(triangle, 0);
  }

  size_t indexBufSize = 0;
  size_t vertBufSize = 0;
  for (Mesh& mesh : model.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertBufSize += mesh.GetVertTable().Size();
  }

  indexBuffer.Resize(indexBufSize);
  vertBuffer.Resize(vertBufSize, stride);
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

void World::LoadOBJ(Model& model, Asset& asset) {
  MemoryDeserializer objDeserializer(asset.Loader());

  OBJFile objFile;
  objFile.Deserialize(objDeserializer);

  model.CreateNewMesh();
  model.SetShadingOrder(objFile.ShadingOrder());
  model.SetStride(objFile.Stride());
  Mesh& mesh = model[0];

#if !DEBUG_FLAT_SHADE_RGB
  bool isTextureMapped = objFile.ShadingOrder().Contains(ShadingMode(ShadingModes::UV, 2));
#else
  bool isTextureMapped = false;
#endif
  const size_t textureStride = 2;

  if (isTextureMapped) {
    Bundle& bundle = Bundle::Get();
    Asset* textureAsset = bundle.GetAsset(objFile.AlbedoTexture());

    if (textureAsset == nullptr) {
      ZAssert(false);
      return;
    }

    model.TextureId() = TexturePool::Get().LoadTexture(*textureAsset);
  }
  else {
    objFile.ShadingOrder().Clear();
    objFile.ShadingOrder().EmplaceBack(ShadingModes::RGB, 3);
    model.SetShadingOrder(objFile.ShadingOrder());
    model.SetStride(7);
  }

  const size_t stride = objFile.Stride();

  size_t vertSize = objFile.Verts().Size() * stride;
  size_t indexSize = objFile.Faces().Size();

  mesh.Resize(vertSize, indexSize);

  if (isTextureMapped) {
    const size_t scratchSize = 6;
    float vertex[scratchSize];

    for (size_t i = 0; i < objFile.Verts().Size(); ++i) {
      const Vec4& vector = objFile.Verts()[i];
      ZAssert(FloatEqual(vector[3], 1.f));

      memcpy(vertex, reinterpret_cast<const float*>(&vector), sizeof(Vec4));
      memcpy(vertex + 4, &objFile.UVs()[i], textureStride * sizeof(float));

      mesh.SetData(vertex, i * stride, stride * sizeof(float));
    }
  }
  else {
    const size_t scratchSize = 7;
    float vertex[scratchSize];

    // Assign 
    const float R[] = { 1.f, 0.f, 0.f };
    const float G[] = { 0.f, 1.f, 0.f };
    const float B[] = { 0.f, 0.f, 1.f };
    for (size_t i = 0, triIndex = 0; i < objFile.Verts().Size(); ++i) {
      const Vec4& vector = objFile.Verts()[i];
      ZAssert(FloatEqual(vector[3], 1.f));

      memcpy(vertex, reinterpret_cast<const float*>(&vector), sizeof(Vec4));

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

      triIndex++;

      mesh.SetData(vertex, i * stride, stride * sizeof(float));
    }
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
