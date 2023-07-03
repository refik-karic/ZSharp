#include "OBJLoader.h"

#include "ZAssert.h"

#include <cstring>
#include "Logger.h"

#include "ZFile.h"

namespace ZSharp {
OBJLoader::OBJLoader(const FileString& path, OBJFile& objFile) : mObjFile(objFile) {
  ParseRaw(path);
}

OBJLoader::OBJLoader(IDeserializer& deserializer, OBJFile& objFile) : mObjFile(objFile) {
  Deserialize(deserializer);
}

void OBJLoader::Serialize(ISerializer& serializer) {
  mObjFile.Verts().Serialize(serializer);
  mObjFile.Normals().Serialize(serializer);
  mObjFile.UVs().Serialize(serializer);
  mObjFile.Faces().Serialize(serializer);
  mObjFile.ShadingOrder().Serialize(serializer);
  mObjFile.AlbedoTexture().Serialize(serializer);
}

void OBJLoader::Deserialize(IDeserializer& deserializer) {
  mObjFile.Verts().Deserialize(deserializer);
  mObjFile.Normals().Deserialize(deserializer);
  mObjFile.UVs().Deserialize(deserializer);
  mObjFile.Faces().Deserialize(deserializer);
  mObjFile.ShadingOrder().Deserialize(deserializer);
  mObjFile.AlbedoTexture().Deserialize(deserializer);
}

void OBJLoader::ParseRaw(const FileString& objFilePath) {
  MemoryMappedFileReader reader(objFilePath);
  if (!reader.IsOpen()) {
    return;
  }

  const char* fileBuffer = reader.GetBuffer();
  size_t fileSize = reader.GetSize();
  size_t lastLineIndex = 0;

  for (size_t offset = 0; offset < fileSize; ++offset) {
    if (fileBuffer[offset] == '\n') {
      size_t lineLength = offset - lastLineIndex;
      ParseLine(fileBuffer + lastLineIndex, lineLength);
      lastLineIndex = offset + 1;
    }
  }
}

void OBJLoader::ParseLine(const char* currentLine, size_t length) {
  const char* rawLine = currentLine;

  // All lines in the input file must be terminated with a newline.
  // The parser will fail to read the last line properly if this is the case.

  switch (rawLine[0]) {
    case 'v':
      if (rawLine[1] == 'n') {
        // Vertex Normals.
        Vec3 vertex;
        String choppedLine(rawLine + 3, 0, length);
        ParseVec3(vertex, choppedLine, 0.0f);
        mObjFile.Normals().PushBack(vertex);
      }
      else if (rawLine[1] == 'p') {
        // Vertex Parameters.
        // For curves and surfaces, ignoring for now.
        String choppedLine(rawLine, 0, length);
        Logger::Log(LogCategory::Info, String::FromFormat("Vertex Parameters: [{0}]\n", choppedLine));
      }
      else if (rawLine[1] == 't') {
        // Vertex Texture Coordinates (U, V, W).
        Vec3 vertex;
        String choppedLine(rawLine + 3, 0, length);
        ParseVec3(vertex, choppedLine, 0.0f);
        mObjFile.UVs().PushBack(vertex);
      }
      else {
        // Vertex Data.
        Vec4 vertex;
        String choppedLine(rawLine + 2, 0, length);
        ParseVec4(vertex, choppedLine, 1.0f);
        mObjFile.Verts().PushBack(vertex);
      }
      break;
    case 'f':
      // Vertex face.
    {
      OBJFace face;
      String choppedLine(rawLine + 2, 0, length);
      ParseFace(face, choppedLine);
      mObjFile.Faces().PushBack(face);
    }
    break;
    case 'l':
    {
      // Line.
      String choppedLine(rawLine, 0, length);
      Logger::Log(LogCategory::Info, String::FromFormat("Line: [{0}]\n", choppedLine));
    }
      break;
    default:
    {
      String choppedLine(rawLine, 0, length);
      Logger::Log(LogCategory::Info, String::FromFormat("Unknown Line: [{0}]\n", choppedLine));
    }
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
      uint64 vertexIndex = line.ToUint64() - 1;
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
      uint64 textureIndex = line.ToUint64() - 1;
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
      uint64 normalIndex = line.ToUint64() - 1;
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
