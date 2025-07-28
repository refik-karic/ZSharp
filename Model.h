#pragma once

#include "ZBaseTypes.h"

#include "IndexBuffer.h"
#include "Mesh.h"
#include "VertexBuffer.h"
#include "PhysicsObject.h"
#include "ISerializable.h"

namespace ZSharp {

class Model final : public PhysicsObject, public ISerializable {
  public:

  Model() = default;

  Model(const Model& copy);

  void operator=(const Model& rhs);

  Mesh& GetMesh();

  const Mesh& GetMesh() const;

  void FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const;

  virtual void Serialize(ISerializer& serializer) override;

  virtual void Deserialize(IDeserializer& deserializer) override;

  private:
  Mesh mMesh;
};

}
