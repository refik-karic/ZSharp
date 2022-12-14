#include "Renderer.h"

#include "ZBaseTypes.h"

#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Model.h"
#include "VertexBuffer.h"
#include "ZColor.h"
#include "ZDrawing.h"

#define DISABLE_DEBUG_TRANSFORMS 1

namespace ZSharp {
Renderer::Renderer() {
}

void Renderer::Initialize() {
  const ZColor colorGreen(ZColors::GREEN);
  mBuffer.Clear(colorGreen);
}

void Renderer::RenderNextFrame(World& world, Camera& camera) {
  const ZColor clearColor(ZColors::ORANGE);
  mBuffer.Clear(clearColor);

  for (size_t i = 0; i < world.GetTotalModels(); ++i) {
    Model& model = world.GetModels()[i];
    VertexBuffer& vertexBuffer = world.GetVertexBuffers()[i];
    IndexBuffer& indexBuffer = world.GetIndexBuffers()[i];

    vertexBuffer.Reset();
    indexBuffer.Reset();
    model.FillBuffers(vertexBuffer, indexBuffer);

#if !DISABLE_DEBUG_TRANSFORMS
    vertexBuffer.ApplyTransform(model.GetRotation());
#endif
    camera.PerspectiveProjection(vertexBuffer, indexBuffer);

    if (mRenderMode) {
      DrawTrianglesFlat(mBuffer, vertexBuffer, indexBuffer);
    }
    else {
      DrawTrianglesWireframe(mBuffer, vertexBuffer, indexBuffer);
    }
  }
}

uint8* Renderer::GetFrame() {
  return mBuffer.GetBuffer();
}

void Renderer::FlipRenderMode() {
  mRenderMode = !mRenderMode;
}

}
