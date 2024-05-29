#pragma once

#include "ZBaseTypes.h"
#include "Mat4x4.h"
#include "AABB.h"

namespace ZSharp {

class VertexBuffer final {
  public:

  VertexBuffer() = default;

  ~VertexBuffer();

  VertexBuffer(const VertexBuffer& rhs);

  void operator=(const VertexBuffer& rhs);

  float* GetInputDataX(int32 index) const;

  float* GetInputDataX(int32 index);

  float* GetInputDataY(int32 index) const;

  float* GetInputDataY(int32 index);

  float* GetInputDataZ(int32 index) const;

  float* GetInputDataZ(int32 index);

  float* GetInputDataW(int32 index) const;

  float* GetInputDataW(int32 index);

  float* GetAttributeData(int32 index, int32 attribute) const;

  float* GetAttributeData(int32 index, int32 attribute);

  const float* GetClipDataX(int32 index) const;

  float* GetClipDataX(int32 index);

  const float* GetClipDataY(int32 index) const;

  float* GetClipDataY(int32 index);

  const float* GetClipDataZ(int32 index) const;

  float* GetClipDataZ(int32 index);

  const float* GetClipDataW(int32 index) const;

  float* GetClipDataW(int32 index);

  float* GetAttributeClipData(int32 index, int32 attribute) const;

  float* GetAttributeClipData(int32 index, int32 attribute);

  int32 GetTotalSize() const;

  int32 GetVertSize() const;

  int32 GetNumAttributes() const;

  void CopyInputData(const float** data, int32 index, int32 length, int32 numAttributes);

  void Resize(int32 vertexSize, int32 numAttributes);

  void Clear();

  void Reset();

  void ApplyTransform(const Mat4x4& transform);

  void AppendClipDataAOS(const float* data, int32 lengthBytes, int32 numVertices, int32 numAttributes);

  int32 GetClipLength() const;

  void ShuffleClippedData();

  AABB ComputeBoundingBox() const;

  bool& WasClipped();

  private:
  float* mXData = nullptr;
  float* mYData = nullptr;
  float* mZData = nullptr;
  float* mWData = nullptr;
  float* mXClipData = nullptr;
  float* mYClipData = nullptr;
  float* mZClipData = nullptr;
  float* mWClipData = nullptr;
  float* mAttributeData = nullptr;
  float* mAttributeClipData = nullptr;
  int32 mInputSize = 0;
  int32 mAllocatedSize = 0;
  int32 mWorkingSize = 0;
  int32 mClipLength = 0;
  int32 mNumAttributes = 0;
  bool mWasClipped = false;
};

}
