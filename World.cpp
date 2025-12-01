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

void World::AssignPlayer(Player* player) {
  mPlayer = player;
}

void World::Load() {
  mLoaded = true;

  if (mPlayer) {
    mDynamicObjects.PushBack(mPlayer);
  }

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
    currentObject->Velocity() += {0.f, GravityPerSecond* (float)deltaMs, 0.f};
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

      LoadModel(model, asset);

      // TODO: Remove me.
      if (asset.Name() == "plane") {
        model.Tag() = PhysicsTag::Static;
      }
      else {
        if (*PhysicsEnabled) {
          model.Position() += {0.f, 15.f, 0.f};
          model.Tag() = PhysicsTag::Dynamic;
        }
        else {
          model.Tag() = PhysicsTag::Static;
        }
      }

      Mesh& mesh = model.GetMesh();
      int32 indexBufSize = (int32)(mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
      int32 vertBufSize = (int32)mesh.GetVertTable().Size();
      int32 vertStride = (int32)mesh.Stride();

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
  Model& model = mActiveModels.EmplaceBack();
  VertexBuffer& vertBuffer = mVertexBuffers.EmplaceBack();
  IndexBuffer& indexBuffer = mIndexBuffers.EmplaceBack();

  Mesh& mesh = model.GetMesh();
  mesh.SetShader(shader);

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

    mesh.TextureId() = GlobalTexturePool->LoadTexture(*textureAsset);
  }

  {
    const int32 strideBytes = stride * sizeof(float);
    int32 vertSize = 3 * stride;
    int32 faceSize = 1;
    mesh.Resize(vertSize, faceSize);
    mesh.SetData(v1, 0, strideBytes);
    mesh.SetData(v2, stride, strideBytes);
    mesh.SetData(v3, stride * 2, strideBytes);
    Triangle triangle(0, stride, stride * 2);
    mesh.SetTriangle(triangle, 0);
  }

  model.BoundingBox() = ComputeBoundingBox(mesh.Stride(), mesh.GetVertTable().GetData(), mesh.GetVertTable().Size());

  int32 indexBufSize = (int32)(mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
  int32 vertBufSize = (int32)mesh.GetVertTable().Size();

  indexBuffer.Resize(indexBufSize);
  vertBuffer.Resize(vertBufSize * stride, stride);
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

void World::LoadModel(Model& model, Asset& asset) {
  MemoryDeserializer meshDeserializer(asset.Loader());

  model.Deserialize(meshDeserializer);
  Mesh& mesh = model.GetMesh();

  const bool isTextureMapped = mesh.GetShader().GetShadingMethod() == ShadingMethod::UV;

  if (isTextureMapped) {
    Bundle* bundle = GlobalBundle;
    Asset* textureAsset = bundle->GetAsset(mesh.AlbedoTexture());

    if (textureAsset == nullptr) {
      ZAssert(false);
      return;
    }

    mesh.TextureId() = GlobalTexturePool->LoadTexture(*textureAsset);
  }
}
}
