#include "Model.h"

#include "ZAssert.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

namespace ZSharp {
Model::Model(const Model& copy) 
  : mMesh(copy.mMesh) {
}

void Model::operator=(const Model& rhs) {
  if (this == &rhs) {
    return;
  }

  mMesh = rhs.mMesh;
}

Mesh& Model::GetMesh() {
  return mMesh;
}

const Mesh& Model::GetMesh() const {
  return mMesh;
}

void Model::FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const {
  NamedScopedTimer(FillBuffers);

  if (mMesh.GetTriangleFaceTable().Size() == 0 || mMesh.GetVertTable().Size() == 0) {
    return;
  }

  indexBuffer.CopyInputData(reinterpret_cast<const int32*>(mMesh.GetTriangleFaceTable().GetData()), 0, (int32)(mMesh.GetTriangleFaceTable().Size() * TRI_VERTS));
  vertexBuffer.CopyInputData(mMesh.GetVertTable().GetData(), 0, (int32)(mMesh.GetVertTable().Size()));
}

void Model::Serialize(ISerializer& serializer) {
  mBoundingBox.Serialize(serializer);
  mMesh.Serialize(serializer);
}

void Model::Deserialize(IDeserializer& deserializer) {
  mBoundingBox.Deserialize(deserializer);
  mMesh.Deserialize(deserializer);
}

}
