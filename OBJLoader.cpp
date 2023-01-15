#include "OBJLoader.h"

#include "ZAssert.h"

#include <cstring>
#include "Logger.h"

#include "ZFile.h"

namespace ZSharp {
OBJLoader::OBJLoader(OBJFile& file, const FileString& path, AssetFormat format)
  : mFile(file) {
  switch (format) {
    case AssetFormat::Raw:
      ParseRaw(path);
      break;
    case AssetFormat::Serialized:
    {
      Deserializer deserializer(path);
      Deserialize(deserializer);
    }
      break;
  }
}

void OBJLoader::Serialize(Serializer& serializer) {
  // Write length of vertex vector
  size_t vertSize = mFile.mVerts.Size();
  serializer.Serialize(&vertSize, sizeof(vertSize));

  // Write vertex vector
  for (Vec4& vector : mFile.mVerts) {
    serializer.Serialize(&vector, sizeof(Vec4));
  }

  // Write length of the normal vector
  size_t normalSize = mFile.mNormals.Size();
  serializer.Serialize(&normalSize, sizeof(size_t));

  // Write normal vector
  for (Vec3& normal : mFile.mNormals) {
    serializer.Serialize(&normal, sizeof(Vec3));
  }

  // Write length of the uv vector
  size_t uvSize = mFile.mUVCoords.Size();
  serializer.Serialize(&uvSize, sizeof(size_t));

  // Write uv vector
  for (Vec3& uv : mFile.mUVCoords) {
    serializer.Serialize(&uv, sizeof(Vec3));
  }

  // Write size of face vector
  size_t faceSize = mFile.mFaces.Size();
  serializer.Serialize(&faceSize, sizeof(size_t));

  // Write face vector
  for (OBJFace& face : mFile.mFaces) {
    serializer.Serialize(&face, sizeof(OBJFace));
  }
}

void OBJLoader::Deserialize(Deserializer& deserializer) {
  // Read vert size
  size_t vertSize = 0;
  deserializer.Deserialize(&vertSize, sizeof(size_t));
  mFile.mVerts.Resize(vertSize);

  // Read verticies
  for (size_t i = 0; i < vertSize; ++i) {
    Vec4 vector;
    deserializer.Deserialize(&vector, sizeof(Vec4));
    mFile.mVerts[i] = vector;
  }

  // Read normal size
  size_t normalSize = 0;
  deserializer.Deserialize(&normalSize, sizeof(size_t));
  mFile.mNormals.Resize(normalSize);

  // Read normals
  for (size_t i = 0; i < normalSize; ++i) {
    Vec3 normal;
    deserializer.Deserialize(&normal, sizeof(Vec3));
    mFile.mNormals[i] = normal;
  }

  // Read uv size
  size_t uvSize = 0;
  deserializer.Deserialize(&uvSize, sizeof(size_t));
  mFile.mUVCoords.Resize(uvSize);

  // Read uvs
  for (size_t i = 0; i < uvSize; ++i) {
    Vec3 uv;
    deserializer.Deserialize(&uv, sizeof(Vec3));
    mFile.mUVCoords[i] = uv;
  }

  // Read face size
  size_t faceSize = 0;
  deserializer.Deserialize(&faceSize, sizeof(size_t));
  mFile.mFaces.Resize(faceSize);

  // Read face vector
  for (size_t i = 0; i < faceSize; ++i) {
    OBJFace face;
    deserializer.Deserialize(&face, sizeof(OBJFace));
    mFile.mFaces[i] = face;
  }
}

void OBJLoader::ParseRaw(const FileString& objFilePath) {
  BufferedFileReader reader(objFilePath);
  if (!reader.IsOpen()) {
    return;
  }

  for (size_t bytesRead = reader.ReadLine(); bytesRead > 0; bytesRead = reader.ReadLine()) {
    if (bytesRead > 2) {
      ParseLine(reader.GetBuffer());
    }
  }
}

void OBJLoader::ParseLine(const char* currentLine) {
  const char* rawLine = currentLine;

  // All lines in the input file must be terminated with a newline.
  // The parser will fail to read the last line properly if this is the case.

  switch (rawLine[0]) {
    case 'v':
      if (rawLine[1] == 'n') {
        // Vertex Normals.
        Vec3 vertex;
        String choppedLine(rawLine + 3);
        ParseVec3(vertex, choppedLine, 0.0f);
        mFile.mNormals.PushBack(vertex);
      }
      else if (rawLine[1] == 'p') {
        // Vertex Parameters.
        // For curves and surfaces, ignoring for now.
        Logger::Log(LogCategory::Info, String::FromFormat("Vertex Parameters: [{0}]\n", rawLine));
      }
      else if (rawLine[1] == 't') {
        // Vertex Texture Coordinates (U, V, W).
        Vec3 vertex;
        String choppedLine(rawLine + 3);
        ParseVec3(vertex, choppedLine, 0.0f);
        mFile.mUVCoords.PushBack(vertex);
      }
      else {
        // Vertex Data.
        Vec4 vertex;
        String choppedLine(rawLine + 2);
        ParseVec4(vertex, choppedLine, 1.0f);
        mFile.mVerts.PushBack(vertex);
      }
      break;
    case 'f':
      // Vertex face.
    {
      OBJFace face;
      String choppedLine(rawLine + 2);
      ParseFace(face, choppedLine);
      mFile.mFaces.PushBack(face);
    }
    break;
    case 'l':
      // Line.
      Logger::Log(LogCategory::Info, String::FromFormat("Line: [{0}]\n", rawLine));
      break;
    default:
      Logger::Log(LogCategory::Info, String::FromFormat("Unknown Line: [{0}]\n", rawLine));
      break;
  }
}

void OBJLoader::ParseVec3(Vec3& fillVec, String& line, float fallback) {
  Vec4 sacrifice;
  ParseVec4(sacrifice, line, fallback);
  memcpy(*fillVec, *sacrifice, sizeof(float) * 3);
}

void OBJLoader::ParseVec4(Vec4& fillVec, String& line, float fallback) {
  for (int32 i = 0; i < 4; ++i) {
    if (!line.IsEmpty()) {
      fillVec[i] = line.ToFloat();
    }
    else {
      fillVec[i] = fallback;
      return;
    }

    const char* delimiter = line.FindFirst(' ');
    if (delimiter == nullptr) {
      line.Clear();
    }
    else {
      line = String(delimiter + 1);
    }
  }
}

void OBJLoader::ParseFace(OBJFace& fillFace, String& line) {
  for (int32 i = 0; i < 3; ++i) {
    if (!line.IsEmpty()) {
      uint64 vertexIndex = line.ToUint64();
      fillFace.triangleFace[i].vertexIndex = vertexIndex;
    }
    else {
      return;
    }

    const char* indexDelimiter = line.FindFirst('/');

    if (indexDelimiter == nullptr) {
      const char* faceDelimiter = line.FindFirst(' ');
      if (faceDelimiter != nullptr) {
        line = String(faceDelimiter + 1);
      }

      continue;
    }

    line = String(indexDelimiter + 1);

    if (!line.IsEmpty()) {
      uint64 textureIndex = line.ToUint64();
      fillFace.triangleFace[i].uvIndex = textureIndex;
    }
    else {
      return;
    }

    indexDelimiter = line.FindFirst('/');

    if (indexDelimiter == nullptr) {
      const char* faceDelimiter = line.FindFirst(' ');
      if (faceDelimiter != nullptr) {
        line = String(faceDelimiter + 1);
      }

      continue;
    }

    line = String(indexDelimiter + 1);

    if (!line.IsEmpty()) {
      uint64 normalIndex = line.ToUint64();
      fillFace.triangleFace[i].normalIndex = normalIndex;
    }
    else {
      return;
    }

    const char* faceDelimiter = line.FindFirst(' ');
    if (faceDelimiter != nullptr) {
      line = String(faceDelimiter + 1);
    }
    else {
      break;
    }
  }
}

}
