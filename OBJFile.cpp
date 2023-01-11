#include "OBJFile.h"

#include <cstring>
#include "Logger.h"

#include "ZFile.h"

namespace ZSharp {
OBJFile::OBJFile(FileString& objFilePath, AssetFormat format) {
  switch (format) {
  case AssetFormat::Raw:
    ParseRaw(objFilePath);
    break;
  case AssetFormat::Serialized:
    Deserialize(objFilePath);
    break;
  }
}

const Array<Vec4>& OBJFile::GetVerts() {
  return mVerts;
}

const Array<OBJFace>& OBJFile::GetFaces() {
  return mFaces;
}

void OBJFile::Serialize(FileString& destPath) {
  BufferedFileWriter fileWriter(destPath, 0);
  if (!fileWriter.IsOpen()) {
    return;
  }

  // Write length of vertex vector
  size_t vertSize = mVerts.Size();
  fileWriter.Write(reinterpret_cast<void*>(&vertSize), sizeof(size_t));

  // Write vertex vector
  for (Vec4& vector : mVerts) {
    fileWriter.Write(reinterpret_cast<void*>(&vector), sizeof(Vec4));
  }

  // Write length of the normal vector
  size_t normalSize = mNormals.Size();
  fileWriter.Write(reinterpret_cast<void*>(&normalSize), sizeof(size_t));

  // Write normal vector
  for (Vec3& normal : mNormals) {
    fileWriter.Write(reinterpret_cast<void*>(&normal), sizeof(Vec3));
  }

  // Write length of the uv vector
  size_t uvSize = mUVCoords.Size();
  fileWriter.Write(reinterpret_cast<void*>(&uvSize), sizeof(size_t));

  // Write uv vector
  for (Vec3& uv : mUVCoords) {
    fileWriter.Write(reinterpret_cast<void*>(&uv), sizeof(Vec3));
  }

  // Write size of face vector
  size_t faceSize = mFaces.Size();
  fileWriter.Write(reinterpret_cast<void*>(&faceSize), sizeof(size_t));

  // Write face vector
  for (OBJFace& face : mFaces) {
    fileWriter.Write(reinterpret_cast<void*>(&face), sizeof(OBJFace));
  }
}

void OBJFile::Deserialize(FileString& objFilePath) {
  BufferedFileReader fileReader(objFilePath);
  if (!fileReader.IsOpen()) {
    return;
  }

  // Read vert size
  size_t vertSize = 0;
  fileReader.Read(reinterpret_cast<void*>(&vertSize), sizeof(size_t));
  mVerts.Resize(vertSize);

  // Read verticies
  for (size_t i = 0; i < vertSize; ++i) {
    Vec4 vector;
    fileReader.Read(reinterpret_cast<void*>(&vector), sizeof(Vec4));
    mVerts[i] = vector;
  }

  // Read normal size
  size_t normalSize = 0;
  fileReader.Read(reinterpret_cast<void*>(&normalSize), sizeof(size_t));
  mNormals.Resize(normalSize);

  // Read normals
  for (size_t i = 0; i < normalSize; ++i) {
    Vec3 normal;
    fileReader.Read(reinterpret_cast<void*>(&normal), sizeof(Vec3));
    mNormals[i] = normal;
  }

  // Read uv size
  size_t uvSize = 0;
  fileReader.Read(reinterpret_cast<void*>(&uvSize), sizeof(size_t));
  mUVCoords.Resize(uvSize);

  // Read uvs
  for (size_t i = 0; i < uvSize; ++i) {
    Vec3 uv;
    fileReader.Read(reinterpret_cast<void*>(&uv), sizeof(Vec3));
    mUVCoords[i] = uv;
  }

  // Read face size
  size_t faceSize = 0;
  fileReader.Read(reinterpret_cast<void*>(&faceSize), sizeof(size_t));
  mFaces.Resize(faceSize);

  // Read face vector
  for (size_t i = 0; i < faceSize; ++i) {
    OBJFace face;
    fileReader.Read(reinterpret_cast<void*>(&face), sizeof(OBJFace));
    mFaces[i] = face;
  }
}

void OBJFile::ParseRaw(FileString& objFilePath) {
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

void OBJFile::ParseLine(const char* currentLine) {
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
      mNormals.PushBack(vertex);
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
      mUVCoords.PushBack(vertex);
    }
    else {
      // Vertex Data.
      Vec4 vertex;
      String choppedLine(rawLine + 2);
      ParseVec4(vertex, choppedLine, 1.0f);
      mVerts.PushBack(vertex);
    }
    break;
  case 'f':
    // Vertex face.
  {
    OBJFace face;
    String choppedLine(rawLine + 2);
    ParseFace(face, choppedLine);
    mFaces.PushBack(face);
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

void OBJFile::ParseVec3(Vec3& fillVec, String& line, float fallback) {
  Vec4 sacrifice;
  ParseVec4(sacrifice, line, fallback);
  memcpy(*fillVec, *sacrifice, sizeof(float) * 3);
}

void OBJFile::ParseVec4(Vec4& fillVec, String& line, float fallback) {
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

void OBJFile::ParseFace(OBJFace& fillFace, String& line) {
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
