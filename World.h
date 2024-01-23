#pragma once

#include "Array.h"
#include "Asset.h"
#include "ConsoleVariable.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "PhysicsObject.h"
#include "ShadingMode.h"
#include "VertexBuffer.h"
#include "PlatformAudio.h"
#include "MP3.h"

namespace ZSharp {
class World final {
  public:

  World();

  ~World();

  void Load();

  void Reload();

  void TickPhysics(size_t deltaMs);

  void TickAudio(size_t deltaMs);

  size_t GetTotalModels() const;

  Array<Model>& GetModels();

  Array<VertexBuffer>& GetVertexBuffers();

  Array<IndexBuffer>& GetIndexBuffers();

  protected:

  private:
  Array<Model> mActiveModels;
  Array<VertexBuffer> mVertexBuffers;
  Array<IndexBuffer> mIndexBuffers;

  Array<PhysicsObject*> mDynamicObjects;
  Array<PhysicsObject*> mStaticObjects;

  PlatformAudioDevice* mAudioDevice = nullptr;
  MP3::PCMAudioFloat mAmbientTrack;
  size_t mAudioPosition = 0;

  ConsoleVariable<void> mWorldReloadVar;

  void DebugLoadTriangle(const float* v1, const float* v2, const float* v3, ShadingModeOrder order, int32 stride);

  void LoadModels();

  void LoadOBJ(Model& model, Asset& asset);
};

}
