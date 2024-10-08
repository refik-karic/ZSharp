﻿#include "Renderer.h"

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

  mDepthBufferDirty = false;

  for (size_t i = 0; i < world.GetTotalModels(); ++i) {
    Model& model = world.GetModels()[i];

    const AABB aabb(AABB::TransformAndRealign(model.BoundingBox(), model.ObjectTransform()));
    ClipBounds clipBounds;
    {
      VertexBuffer aabbVertexBuffer;
      IndexBuffer aabbIndexBuffer;
      TriangulateAABB(aabb, aabbVertexBuffer, aabbIndexBuffer);

      Mat4x4 identity;
      identity.Identity();

      clipBounds = camera.ClipBoundsCheck(aabbVertexBuffer, aabbIndexBuffer, identity);

      if (*VisualizeAABB) {
        const ZColor aabbColor(ZColors::GREEN);

        aabbVertexBuffer.Clear();
        aabbIndexBuffer.Clear();
        TriangulateAABBWithColor(aabb, aabbVertexBuffer, aabbIndexBuffer, aabbColor);

        camera.PerspectiveProjection(aabbVertexBuffer, aabbIndexBuffer, clipBounds, identity);
        DrawTrianglesWireframe(mFramebuffer, aabbVertexBuffer, aabbIndexBuffer, aabbVertexBuffer.WasClipped());
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
      case RenderMode::FLAT:
      {
        switch (shader.GetShadingMethod()) {
          case ShadingMethod::RGB:
          {
            DrawTrianglesFlatRGB(mFramebuffer, mDepthBuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped());
          }
            break;
          case ShadingMethod::UV:
          {
            Texture* texture = TexturePool::Get().GetTexture(model.TextureId());
            DrawTrianglesFlatUV(mFramebuffer, mDepthBuffer, vertexBuffer, indexBuffer, vertexBuffer.WasClipped(), texture, *MipOverride);
          }
          break;
          default:
            break;
        }
      }
        break;
      case RenderMode::WIREFRAME:
      {
        switch (shader.GetShadingMethod()) {
          case ShadingMethod::RGB:
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
  mFramebuffer.Clear(color, 0, mFramebuffer.GetSize());
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

DepthBuffer& Renderer::GetDepthBuffer() {
  return mDepthBuffer;
}

}
