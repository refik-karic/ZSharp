#include "Renderer.h"

#include "ZBaseTypes.h"

#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Model.h"
#include "VertexBuffer.h"
#include "ZColor.h"
#include "ZDrawing.h"
#include "ScopedTimer.h"

namespace ZSharp {
Renderer::Renderer() {
}

void Renderer::RenderNextFrame(World& world, Camera& camera) {
  NamedScopedTimer(RenderFrame);

  const ZColor clearColor(ZColors::ORANGE);
  mBuffer.Clear(clearColor);

  for (size_t i = 0; i < world.GetTotalModels(); ++i) {
    Model& model = world.GetModels()[i];
    VertexBuffer& vertexBuffer = world.GetVertexBuffers()[i];
    IndexBuffer& indexBuffer = world.GetIndexBuffers()[i];

    vertexBuffer.Reset();
    indexBuffer.Reset();
    model.FillBuffers(vertexBuffer, indexBuffer);

    vertexBuffer.ApplyTransform(model.ObjectTransform());

    camera.PerspectiveProjection(vertexBuffer, indexBuffer);

    switch (mRenderMode) {
      case RenderMode::FLAT:
        DrawTrianglesFlat(mBuffer, vertexBuffer, indexBuffer, model.ShadingOrder());
        break;
      case RenderMode::WIREFRAME:
        DrawTrianglesWireframe(mBuffer, vertexBuffer, indexBuffer);
        break;
    }
  }
}

uint8* Renderer::GetFrame() {
  return mBuffer.GetBuffer();
}

void Renderer::ToggleRenderMode(RenderMode mode) {
  mRenderMode = mode;
}

}
