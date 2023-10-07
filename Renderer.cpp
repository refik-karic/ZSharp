﻿#include "Renderer.h"

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

#include <cmath>

#define VISUALIZE_AABB 1

namespace ZSharp {
Renderer::Renderer() {
}

void Renderer::RenderNextFrame(World& world, Camera& camera) {
  NamedScopedTimer(RenderFrame);

  const ZColor clearColor(ZColors::ORANGE);
  mFramebuffer.Clear(clearColor);
  mDepthBuffer.Clear();

  for (size_t i = 0; i < world.GetTotalModels(); ++i) {
    Model& model = world.GetModels()[i];
    VertexBuffer& vertexBuffer = world.GetVertexBuffers()[i];
    IndexBuffer& indexBuffer = world.GetIndexBuffers()[i];

    vertexBuffer.Reset();
    indexBuffer.Reset();
    model.FillBuffers(vertexBuffer, indexBuffer);

    vertexBuffer.ApplyTransform(model.ObjectTransform());

    /* 
      TODO: It would be significantly more efficient to precompute the AABB once (either at load or from bundle).
        If the model scales/translates/rotates after the fact, we should transform the AABB and then correct it to be axis aligned again.
    */

    const AABB aabb(vertexBuffer.ComputeBoundingBox());
    ClipBounds clipBounds;
    {
      VertexBuffer aabbVertexBuffer;
      IndexBuffer aabbIndexBuffer;
      TriangulateAABB(aabb, aabbVertexBuffer, aabbIndexBuffer);

      clipBounds = camera.ClipBoundsCheck(aabbVertexBuffer, aabbIndexBuffer);

#if VISUALIZE_AABB
      aabbVertexBuffer.Clear();
      aabbIndexBuffer.Clear();
      TriangulateAABB(aabb, aabbVertexBuffer, aabbIndexBuffer);

      camera.PerspectiveProjection(aabbVertexBuffer, aabbIndexBuffer, clipBounds);
      DrawTrianglesWireframe(mFramebuffer, aabbVertexBuffer, aabbIndexBuffer);
#endif
    }

    camera.PerspectiveProjection(vertexBuffer, indexBuffer, clipBounds);

    // TODO: This is a little hacky for now. We want a cleaner solution to handle missing textures.
    Texture* texture = &model.GetTexture();
    if (!texture->IsAssigned()) {
      texture = nullptr;
    }

    switch (mRenderMode) {
      case RenderMode::FLAT:
        DrawTrianglesFlat(mFramebuffer, mDepthBuffer, vertexBuffer, indexBuffer, model.ShadingOrder(), texture);
        break;
      case RenderMode::WIREFRAME:
        DrawTrianglesWireframe(mFramebuffer, vertexBuffer, indexBuffer);
        break;
    }
  }
}

uint8* Renderer::GetFrame() {
  return mFramebuffer.GetBuffer();
}

uint8* Renderer::GetDepth() {
  size_t width = mDepthBuffer.GetWidth();
  size_t height = mDepthBuffer.GetHeight();
  float* buffer = mDepthBuffer.GetBuffer();

  ZColor black(ZColors::BLACK);
  ZColor white(ZColors::WHITE);

  // TODO: Find a better way to scale the depth values.
  //  This is just a good guess.
  float denominator = -6.f;

  for (size_t h = 0; h < height; ++h) {
    for (size_t w = 0; w < width; ++w) {
      float* pixel = buffer + (h * width) + w;

      float numerator = *pixel - 6.f;
      float t = numerator / denominator;

      ZColor pixelColor(black, white, t);
      (*((uint32*)pixel)) = pixelColor.Color();
    }
  }

  return reinterpret_cast<uint8*>(buffer);
}

void Renderer::ToggleRenderMode(RenderMode mode) {
  mRenderMode = mode;
}

Framebuffer& Renderer::GetFrameBuffer() {
  return mFramebuffer;
}

}
