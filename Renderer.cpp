#include "Renderer.h"

#include "ZBaseTypes.h"

#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Model.h"
#include "VertexBuffer.h"
#include "ZColor.h"
#include "ZConfig.h"
#include "ZDrawing.h"

namespace ZSharp {
Renderer::Renderer() {
}

void Renderer::Initialize() {
  ZConfig& config = ZConfig::GetInstance();
  mBuffer.Initialize(config.GetViewportWidth(), config.GetViewportHeight(), config.GetViewportStride());

  const ZColor colorGreen{ ZColors::GREEN };
  mBuffer.Clear(colorGreen);
}

void Renderer::RenderNextFrame(World& world, Camera& camera) {
  ZConfig& config = ZConfig::GetInstance();

  if (config.SizeChanged(mBuffer.GetWidth(), mBuffer.GetHeight())) {
    mBuffer.Resize();
    camera.Resize();
  }

  const ZColor colorRed{ZColors::RED};
  const ZColor colorBlue{ZColors::BLUE};

  mBuffer.Clear(colorBlue);

  for (size_t i = 0; i < world.GetTotalModels(); ++i) {
    Model& model = world.GetModels()[i];
    VertexBuffer& vertexBuffer = world.GetVertexBuffers()[i];
    IndexBuffer& indexBuffer = world.GetIndexBuffers()[i];

    vertexBuffer.Reset();
    indexBuffer.Reset();
    model.FillBuffers(vertexBuffer, indexBuffer);

    vertexBuffer.ApplyTransform(model.GetRotation());
    camera.PerspectiveProjection(vertexBuffer, indexBuffer);

    if (mRenderMode) {
      DrawTrianglesFlat(mBuffer, vertexBuffer, indexBuffer, colorRed);
    }
    else {
      DrawTrianglesWireframe(mBuffer, vertexBuffer, indexBuffer, colorRed);
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
