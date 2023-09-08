#include "Renderer.h"

#include "ZBaseTypes.h"

#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Model.h"
#include "VertexBuffer.h"
#include "ZColor.h"
#include "ZDrawing.h"
#include "ScopedTimer.h"
#include "DebugText.h"
#include "ZAlgorithm.h"

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

    const AABB aabb(model.ComputeBoundingBox());
    ClipBounds clipBounds;
    {
      VertexBuffer aabbVertexBuffer;
      IndexBuffer aabbIndexBuffer;
      TriangulateAABB(aabb, aabbVertexBuffer, aabbIndexBuffer);
      aabbVertexBuffer.ApplyTransform(model.ObjectTransform());

      clipBounds = camera.ClipBoundsCheck(aabbVertexBuffer, aabbIndexBuffer);
    }

    camera.PerspectiveProjection(vertexBuffer, indexBuffer, clipBounds);

    // TODO: This is a little hacky for now. We want a cleaner solution to handle missing textures.
    Texture* texture = &model.GetTexture();
    if (!texture->IsAssigned()) {
      texture = nullptr;
    }

    switch (mRenderMode) {
      case RenderMode::FLAT:
        DrawTrianglesFlat(mBuffer, vertexBuffer, indexBuffer, model.ShadingOrder(), texture);
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

Framebuffer& Renderer::GetFrameBuffer() {
  return mBuffer;
}

}
