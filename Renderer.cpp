#include "Renderer.h"

#include "ZBaseTypes.h"

#include "ConsoleVariable.h"
#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Model.h"
#include "VertexBuffer.h"
#include "ZColor.h"
#include "ZDrawing.h"
#include "ScopedTimer.h"
#include "DebugText.h"
#include "ZAlgorithm.h"
#include "PlatformIntrinsics.h"
#include "TexturePool.h"

#include <cmath>
#include <cstring>

namespace ZSharp {
ConsoleVariable<int32> DevRenderMode("RenderMode", 1);

ConsoleVariable<bool> VisualizeAABB("VizAABB", false);

ConsoleVariable<int32> MipOverride("MipOverride", 4);

ConsoleVariable<ZColor> WireframeColor("WireframeColor", ZColor(ZColors::GREEN));

Renderer::Renderer() {
  mAABBVertexBuffer.Resize(8 * 4, 4);
  mAABBIndexBuffer.Resize(12 * 3);
}

void Renderer::RenderNextFrame(World& world, Camera& camera) {
  NamedScopedTimer(RenderFrame);

  if (*DevRenderMode) {
    mRenderMode = RenderMode::FILL;
  }
  else {
    mRenderMode = RenderMode::WIREFRAME;
  }

  mDepthBufferDirty = false;

  for (size_t i = 0; i < world.GetTotalModels(); ++i) {
    Model& model = world.GetModels()[i];

    const AABB aabb(AABB::TransformAndRealign(model.BoundingBox(), model.ObjectTransform()));
    ClipBounds clipBounds;
    {
      mAABBVertexBuffer.Reset();
      mAABBIndexBuffer.Reset();
      TriangulateAABB(aabb, mAABBVertexBuffer, mAABBIndexBuffer, false);

      Mat4x4 identity;
      identity.Identity();

      clipBounds = camera.ClipBoundsCheck(mAABBVertexBuffer, mAABBIndexBuffer, identity);

      if (*VisualizeAABB) {
        mAABBVertexBuffer.Reset();
        mAABBIndexBuffer.Reset();
        TriangulateAABB(aabb, mAABBVertexBuffer, mAABBIndexBuffer, false);
        camera.PerspectiveProjection(mAABBVertexBuffer, mAABBIndexBuffer, clipBounds, identity);
        WireframeShader(mFramebuffer, mAABBVertexBuffer, mAABBIndexBuffer, mAABBVertexBuffer.WasClipped(), *WireframeColor);
      }
    }

    if (clipBounds == ClipBounds::Outside) {
      continue;
    }

    VertexBuffer& vertexBuffer = world.GetVertexBuffers()[i];
    IndexBuffer& indexBuffer = world.GetIndexBuffers()[i];

    vertexBuffer.Reset();
    indexBuffer.Reset();

    model.FillBuffers(vertexBuffer, indexBuffer);

    camera.PerspectiveProjection(vertexBuffer, indexBuffer, clipBounds, model.ObjectTransform());

    const ShaderDefinition& shader = model.GetShader();

    switch (mRenderMode) {
      case RenderMode::FILL:
      {
        switch (shader.GetShadingMethod()) {
          case ShadingMethod::RGB:
          {
            RGBShader(mFramebuffer, mDepthBuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped());
          }
            break;
          case ShadingMethod::UV:
          {
            Texture* texture = TexturePool::Get().GetTexture(model.TextureId());
            TextureMappedShader(mFramebuffer, mDepthBuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped(), texture, *MipOverride);
          }
          break;
          default:
            break;
        }
      }
        break;
      case RenderMode::WIREFRAME:
      {
        WireframeShader(mFramebuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped(), *WireframeColor);
      }
        break;
    }
  }
}

uint8* Renderer::GetFrame() {
  return mFramebuffer.GetBuffer();
}

uint8* Renderer::GetDepth() {
  float* buffer = mDepthBuffer.GetBuffer();

  if (!mDepthBufferDirty) {
    mDepthBufferDirty = true;
  }
  else {
    return reinterpret_cast<uint8*>(buffer);
  }

  NamedScopedTimer(DepthBufferVisualize);

  size_t width = mDepthBuffer.GetWidth();
  size_t height = mDepthBuffer.GetHeight();

  DepthBufferVisualizeImpl(buffer, width, height);
  return reinterpret_cast<uint8*>(buffer);
}

void Renderer::ClearFramebuffer(const ZColor& color) {
  mFramebuffer.Clear(color, 0, mFramebuffer.GetSize());
}

void Renderer::ToggleRenderMode(RenderMode mode) {
  if (mode == RenderMode::FILL) {
    *DevRenderMode = 1;
  }
  else {
    *DevRenderMode = 0;
  }
}

Framebuffer& Renderer::GetFrameBuffer() {
  return mFramebuffer;
}

DepthBuffer& Renderer::GetDepthBuffer() {
  return mDepthBuffer;
}

}
