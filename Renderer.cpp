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

Renderer::Renderer() {
}

void Renderer::RenderNextFrame(World& world, Camera& camera) {
  NamedScopedTimer(RenderFrame);

  if (*DevRenderMode) {
    mRenderMode = RenderMode::FLAT;
  }
  else {
    mRenderMode = RenderMode::WIREFRAME;
  }

  const ZColor clearColor(ZColors::ORANGE);
  mFramebuffer.Clear(clearColor);
  mDepthBuffer.Clear();
  mDepthBufferDirty = false;

  for (size_t i = 0; i < world.GetTotalModels(); ++i) {
    Model& model = world.GetModels()[i];
    VertexBuffer& vertexBuffer = world.GetVertexBuffers()[i];
    IndexBuffer& indexBuffer = world.GetIndexBuffers()[i];

    vertexBuffer.Reset();
    indexBuffer.Reset();
    model.FillBuffers(vertexBuffer, indexBuffer);

    vertexBuffer.ApplyTransform(model.ObjectTransform());

    const AABB aabb(AABB::TransformAndRealign(model.BoundingBox(), model.ObjectTransform()));
    ClipBounds clipBounds;
    {
      VertexBuffer aabbVertexBuffer;
      IndexBuffer aabbIndexBuffer;
      TriangulateAABB(aabb, aabbVertexBuffer, aabbIndexBuffer);

      clipBounds = camera.ClipBoundsCheck(aabbVertexBuffer, aabbIndexBuffer);

      if (*VisualizeAABB) {
        const ZColor aabbColor(ZColors::GREEN);

        aabbVertexBuffer.Clear();
        aabbIndexBuffer.Clear();
        TriangulateAABBWithColor(aabb, aabbVertexBuffer, aabbIndexBuffer, aabbColor);

        camera.PerspectiveProjection(aabbVertexBuffer, aabbIndexBuffer, clipBounds);
        DrawTrianglesWireframe(mFramebuffer, aabbVertexBuffer, aabbIndexBuffer, aabbVertexBuffer.WasClipped());
      }
    }

    camera.PerspectiveProjection(vertexBuffer, indexBuffer, clipBounds);

    switch (mRenderMode) {
      case RenderMode::FLAT:
      {
        switch (model.ShadingOrder()[0].mode) {
          case ShadingModes::RGB:
          {
            DrawTrianglesFlatRGB(mFramebuffer, mDepthBuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped());
          }
            break;
          case ShadingModes::UV:
          {
            Texture* texture = TexturePool::Get().GetTexture(model.TextureId());
            DrawTrianglesFlatUV(mFramebuffer, mDepthBuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped(), texture);
          }
          break;
          default:
            break;
        }
      }
        break;
      case RenderMode::WIREFRAME:
      {
        switch (model.ShadingOrder()[0].mode) {
          case ShadingModes::RGB:
          {
            DrawTrianglesWireframe(mFramebuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped());
          }
          break;
          default:
          {
            ZColor wireframeColor(ZColors::BLUE);
            DrawTrianglesWireframe(mFramebuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped(), wireframeColor);
          }
            break;
        }
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

  Aligned_DepthBufferVisualize(buffer, width, height);
  return reinterpret_cast<uint8*>(buffer);
}

void Renderer::ClearFramebuffer(const ZColor& color) {
  mFramebuffer.Clear(color);
}

void Renderer::ToggleRenderMode(RenderMode mode) {
  if (mode == RenderMode::FLAT) {
    *DevRenderMode = 1;
  }
  else {
    *DevRenderMode = 0;
  }
}

Framebuffer& Renderer::GetFrameBuffer() {
  return mFramebuffer;
}

}
