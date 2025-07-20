#include "World.h"

#include "Bundle.h"
#include "CommonMath.h"
#include "Delegate.h"
#include "Logger.h"
#include "OBJFile.h"
#include "PhysicsAlgorithms.h"
#include "PlatformMemory.h"
#include "ScopedTimer.h"
#include "TexturePool.h"
#include "ZConfig.h"

#include <cstring>

namespace ZSharp {
ConsoleVariable<bool> PhysicsEnabled("PhysicsEnabled", true);
ConsoleVariable<bool> PhysicsForcesEnabled("PhysicsForcesEnabled", true);

ConsoleVariable<bool> DebugModelsRGB("DebugModelsRGB", false);
ConsoleVariable<bool> DebugAudio("DebugAudio", false);
ConsoleVariable<bool> DebugTriangle("DebugTriangle", false);
ConsoleVariable<bool> DebugTriangleTex("DebugTriangleTex", false);

World::World() 
  : mWorldReloadVar("WorldReload", Delegate<void>::FromMember<World, &World::Reload>(this)) {
}

World::~World() {
  if (mAmbientTrack.data != nullptr) {
    PlatformFree(mAmbientTrack.data);
  }

  if (mAudioDevice != nullptr) {
    PlatformReleaseAudioDevice(mAudioDevice);
  }
}

bool World::IsLoaded() {
  return mLoaded;
}

void World::Load() {
  mLoaded = true;

  if (*DebugAudio) {
    FileString audioPath(PlatformGetUserDesktopPath());
    audioPath.SetFilename("AmbientTest.mp3");
    MP3 audioFile(audioPath);

    mAmbientTrack = audioFile.DecompressFloat();
    if (mAmbientTrack.data != nullptr) {
      mAudioDevice = PlatformInitializeAudioDevice(mAmbientTrack.samplesPerSecond, mAmbientTrack.channels, 10);
      ZAssert(mAudioDevice != nullptr);
    }
  }

  if (*DebugTriangle) {
    const float X = 5.f;
    const float Y = 5.f;
    const float Z = 0.f;
    const float W = 1.f;
    const float v1[]{ -X, 0.f, Z, W, 1.f, 0.f, 0.f, 0.f };
    const float v2[]{ 0.f, Y, Z, W, 0.0f, 1.f, 0.f, 0.f };
    const float v3[]{ X, 0.f, Z, W, 0.0f, 0.f, 1.f, 0.f };

    ShaderDefinition shader(4, 4, ShadingMethod::RGB);
    DebugLoadTriangle(v3, v2, v1, shader, 8);
  }
  else if (*DebugTriangleTex) {
    const float X = 5.f;
    const float Y = 5.f;
    const float Z = 0.f;
    const float W = 1.f;
    const float v1[]{ -X, 0.f, Z, W, 1.f, 1.f, 0.f, 0.f };
    const float v2[]{ 0.f, Y, Z, W, 0.f, 1.f, 0.f, 0.f };
    const float v3[]{ X, 0.f, Z, W, 0.5f, 0.f, 0.f, 0.f };

    ShaderDefinition shader(4, 4, ShadingMethod::UV);
    DebugLoadTriangle(v3, v2, v1, shader, 8);
  }
  else {
    ZConfig* config = GlobalConfig;
    if (!config->GetAssetPath().GetAbsolutePath().IsEmpty()) {
      LoadModels();
    }
    else {
      Logger::Log(LogCategory::Warning, "Asset path was empty. Loading debug triangle.\n");

      const float X = 5.f;
      const float Y = 5.f;
      const float Z = 0.f;
      const float W = 1.f;
      const float v1[]{ -X, 0.f, Z, W, 0.f, 1.f, 0.f, 0.f };
      const float v2[]{ 0.f, Y, Z, W, 1.0f, 0.f, 0.f, 0.f };
      const float v3[]{ X, 0.f, Z, W, 0.0f, 0.f, 1.f, 0.f };

      ShaderDefinition shader(4, 4, ShadingMethod::RGB);
      DebugLoadTriangle(v3, v2, v1, shader, 8);
    }
  }
}

void World::Reload() {
  mActiveModels.Clear();
  mVertexBuffers.Clear();
  mIndexBuffers.Clear();

  mDynamicObjects.Clear();
  mStaticObjects.Clear();

  mAudioPosition = 0;

  if (mAmbientTrack.data != nullptr) {
    PlatformFree(mAmbientTrack.data);
  }

  if (mAudioDevice != nullptr) {
    PlatformReleaseAudioDevice(mAudioDevice);
  }

  Load();
}

void World::TickPhysics(size_t deltaMs) {
  NamedScopedTimer(WorldTickPhysics);

  Logger::Log(LogCategory::Perf, String::FromFormat("Ticking physics simulation for {0}ms.\n", deltaMs));

  if (!(*PhysicsEnabled)) {
    return;
  }

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

  if (*PhysicsForcesEnabled) {
    // Update positions for the current timestep.
    for (PhysicsObject*& currentObject : mDynamicObjects) {
      currentObject->Position() += currentObject->Velocity();
    }
  }
}

void World::TickAudio(size_t deltaMs) {
  if (mAudioDevice != nullptr && mAmbientTrack.data != nullptr) {
    // TODO: This forces the track to loop.
    if (mAudioPosition >= mAmbientTrack.length) {
      mAudioPosition = 0;
    }

    // TODO: Audio is still a little choppy, fix this.
    mAudioPosition += PlatformPlayAudio(mAudioDevice, mAmbientTrack.data, mAudioPosition, mAmbientTrack.length, deltaMs);
  }
}

void World::LoadModels() {
  Bundle* bundle = GlobalBundle;
  if (bundle->Assets().Size() == 0) {
    return;
  }

  for (Asset& asset : bundle->Assets()) {
    if (asset.Type() == AssetType::Model) {
      Model& model = mActiveModels.EmplaceBack();
      VertexBuffer& vertBuffer = mVertexBuffers.EmplaceBack();
      IndexBuffer& indexBuffer = mIndexBuffers.EmplaceBack();

      LoadOBJ(model, asset);

      // TODO: Remove me.
      if (asset.Name() == "plane") {
        model.Tag() = PhysicsTag::Static;
      }
      else {
        if (*PhysicsEnabled) {
          model.Position() += Vec3(0.f, 15.f, 0.f);
          model.Tag() = PhysicsTag::Dynamic;
        }
        else {
          model.Tag() = PhysicsTag::Static;
        }
      }

      int32 indexBufSize = 0;
      int32 vertBufSize = 0;
      int32 vertStride = 0;
      for (Mesh& mesh : model.GetMeshData()) {
        indexBufSize += (int32)(mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
        vertBufSize += (int32)mesh.GetVertTable().Size();
        vertStride = (int32)mesh.Stride();
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

void World::DebugLoadTriangle(const float* v1, const float* v2, const float* v3, const ShaderDefinition& shader, int32 stride) {
  Model& model = mActiveModels.EmplaceBack(shader, stride);
  VertexBuffer& vertBuffer = mVertexBuffers.EmplaceBack();
  IndexBuffer& indexBuffer = mIndexBuffers.EmplaceBack();

  model.CreateNewMesh();

  const bool isTextureMapped = shader.GetShadingMethod() == ShadingMethod::UV;

  if (isTextureMapped) {
    Bundle* bundle = GlobalBundle;
    if (bundle->Assets().Size() == 0) {
      return;
    }

    Asset* textureAsset = bundle->GetAsset("wall_256");

    if (textureAsset == nullptr) {
      ZAssert(false);
      return;
    }

    model.TextureId() = GlobalTexturePool->LoadTexture(*textureAsset);
  }

  {
    const int32 strideBytes = stride * sizeof(float);
    Mesh& firstMesh = model[0];
    int32 vertSize = 3 * stride;
    int32 faceSize = 1;
    firstMesh.Resize(vertSize, faceSize);
    firstMesh.SetData(v1, 0, strideBytes);
    firstMesh.SetData(v2, stride, strideBytes);
    firstMesh.SetData(v3, stride * 2, strideBytes);
    Triangle triangle(0 * stride, 1 * stride, 2 * stride);
    firstMesh.SetTriangle(triangle, 0);
  }

  model.BoundingBox() = model.ComputeBoundingBox();

  int32 indexBufSize = 0;
  int32 vertBufSize = 0;
  for (Mesh& mesh : model.GetMeshData()) {
    indexBufSize += (int32)(mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertBufSize += (int32)mesh.GetVertTable().Size();
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
  model.SetShader(objFile.Shader());
  model.SetStride(objFile.Stride());
  model.BoundingBox() = objFile.BoundingBox();
  Mesh& mesh = model[0];

  bool isTextureMapped = objFile.Shader().GetShadingMethod() == ShadingMethod::UV;

  if (*DebugModelsRGB) {
    isTextureMapped = false;
  }

  int32 stride = objFile.Stride();

  if (isTextureMapped) {
    Bundle* bundle = GlobalBundle;
    Asset* textureAsset = bundle->GetAsset(objFile.AlbedoTexture());

    if (textureAsset == nullptr) {
      ZAssert(false);
      return;
    }

    model.TextureId() = GlobalTexturePool->LoadTexture(*textureAsset);
  }
  else {
    ShaderDefinition rbgShader(4, 4, ShadingMethod::RGB);
    model.SetShader(rbgShader);
    model.SetStride(8);
    stride = 8;
  }

  int32 vertSize = (int32)objFile.Verts().Size() * stride;
  int32 indexSize = (int32)objFile.Faces().Size();

  mesh.Resize(vertSize, indexSize);

  if (isTextureMapped) {
    const int32 numVerts = (int32)objFile.Verts().Size();
    const float* vertData = (const float*)objFile.Verts().GetData();
    const float* uvData = (const float*)objFile.UVs().GetData();

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
    const float R[] = { 1.f, 0.f, 0.f };
    const float G[] = { 0.f, 1.f, 0.f };
    const float B[] = { 0.f, 0.f, 1.f };

    const int32 numVerts = (int32)objFile.Verts().Size();
    const float* vertData = (const float*)objFile.Verts().GetData();

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

  const Array<OBJFace>& faceList = objFile.Faces();
  for (int32 triIndex = 0; triIndex < indexSize; ++triIndex) {
    Triangle triangle(static_cast<int32>(faceList[triIndex].triangleFace[0].vertexIndex) * stride,
      static_cast<int32>(faceList[triIndex].triangleFace[1].vertexIndex) * stride,
      static_cast<int32>(faceList[triIndex].triangleFace[2].vertexIndex) * stride
    );
    mesh.SetTriangle(triangle, triIndex);
  }
}
}
