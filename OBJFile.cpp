#include "OBJFile.h"

#include "ZAssert.h"
#include "Logger.h"
#include "ZFile.h"

#include <cstring>

namespace ZSharp {
OBJFile::OBJFile() {
}

void OBJFile::LoadFromFile(const FileString& path) {
  ParseRaw(path);
}

void OBJFile::Serialize(MemorySerializer& serializer) {
  mVerts.Serialize(serializer);
  mNormals.Serialize(serializer);
  mUVCoords.Serialize(serializer);
  mFaces.Serialize(serializer);
  mShadingOrder.Serialize(serializer);
  mAlbedoTexture.Serialize(serializer);
}

void OBJFile::Deserialize(MemoryDeserializer& deserializer) {
  mVerts.Deserialize(deserializer);
  mNormals.Deserialize(deserializer);
  mUVCoords.Deserialize(deserializer);
  mFaces.Deserialize(deserializer);
  mShadingOrder.Deserialize(deserializer);
  mAlbedoTexture.Deserialize(deserializer);
}

Array<Vec4>& OBJFile::Verts() {
  return mVerts;
}

Array<Vec3>& OBJFile::Normals() {
  return mNormals;
}

Array<Vec3>& OBJFile::UVs() {
  return mUVCoords;
}

Array<OBJFace>& OBJFile::Faces() {
  return mFaces;
}

size_t OBJFile::Stride() const {
  size_t stride = 4;
  for (ShadingMode& mode : mShadingOrder) {
    stride += mode.length;
  }

  return stride;
}

ShadingModeOrder& OBJFile::ShadingOrder() {
  return mShadingOrder;
}

String& OBJFile::AlbedoTexture() {
  return mAlbedoTexture;
}

void OBJFile::ParseRaw(const FileString& objFilePath) {
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
      ParseOBJLine(fileBuffer + lastLineIndex, lineLength, objFilePath);
      lastLineIndex = offset + 1;
    }
  }
}

void OBJFile::ParseOBJLine(const char* currentLine, size_t length, const FileString& objFilePath) {
  const char* rawLine = currentLine;

  // All lines in the input file must be terminated with a newline.
  // The parser will fail to read the last line properly if this is the case.

  switch (rawLine[0]) {
    case 'v':
      if (rawLine[1] == 'n') {
        // Vertex Normals.
        float vertex[3];
        String choppedLine(rawLine + 3, 0, length - 3);
        ParseVec3(vertex, choppedLine, 0.0f);
        mNormals.EmplaceBack(vertex);
      }
      else if (rawLine[1] == 'p') {
        // Vertex Parameters.
        // For curves and surfaces, ignoring for now.
        String choppedLine(rawLine, 0, length);
        Logger::Log(LogCategory::Info, String::FromFormat("Vertex Parameters: [{0}]\n", choppedLine));
      }
      else if (rawLine[1] == 't') {
        // Vertex Texture Coordinates (U, V, W).
        float vertex[3];
        String choppedLine(rawLine + 3, 0, length - 3);
        ParseVec3(vertex, choppedLine, 0.0f);
        mUVCoords.EmplaceBack(vertex);
      }
      else {
        // Vertex Data.
        float vertex[4];
        String choppedLine(rawLine + 2, 0, length - 2);
        ParseVec4(vertex, choppedLine, 1.0f);
        mVerts.EmplaceBack(vertex);
      }
      break;
    case 'f':
      // Vertex face.
    {
      OBJFace face;
      String choppedLine(rawLine + 2, 0, length - 2);
      ParseFace(face, choppedLine);
      mFaces.PushBack(face);
    }
    break;
    case 'l':
    {
      // Line.
      String choppedLine(rawLine, 0, length);
      Logger::Log(LogCategory::Info, String::FromFormat("Line: [{0}]\n", choppedLine));
    }
    break;
    case 'm':
    {
      String choppedLine(rawLine + 7, 0, length - 7);
      ParseMaterial(choppedLine, objFilePath);
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

void OBJFile::ParseMTLLine(const char* currentLine, size_t length, const FileString& objFilePath) {
  (void)objFilePath;
  const char* rawLine = currentLine;

  // All lines in the input file must be terminated with a newline.
  // The parser will fail to read the last line properly if this is the case.

  switch (rawLine[0]) {
    case 'N':
    {
      if (rawLine[1] == 's') {
        // Specular component weight.
        String choppedLine(rawLine + 3, 0, length - 3);
        Logger::Log(LogCategory::Info, String::FromFormat("Specular Weight: [{0}]\n", choppedLine));
      }
      else if (rawLine[1] == 'i') {
        // Refraction.
        String choppedLine(rawLine + 3, 0, length - 3);
        Logger::Log(LogCategory::Info, String::FromFormat("Refraction: [{0}]\n", choppedLine));
      }
      else {
        String choppedLine(rawLine + 3, 0, length - 3);
        Logger::Log(LogCategory::Info, String::FromFormat("Unknown Line: [{0}]\n", choppedLine));
      }
    }
      break;
    case 'K':
    {
      if (rawLine[1] == 'a') {
        // Ambient.
        String choppedLine(rawLine + 3, 0, length - 3);
        Logger::Log(LogCategory::Info, String::FromFormat("Ambient: [{0}]\n", choppedLine));
      }
      else if (rawLine[1] == 'd') {
        // Diffuse.
        String choppedLine(rawLine + 3, 0, length - 3);
        Logger::Log(LogCategory::Info, String::FromFormat("Diffuse: [{0}]\n", choppedLine));
      }
      else if (rawLine[1] == 's') {
        // Specular.
        String choppedLine(rawLine + 3, 0, length - 3);
        Logger::Log(LogCategory::Info, String::FromFormat("Specular: [{0}]\n", choppedLine));
      }
      else if (rawLine[1] == 'e') {
        // Emissive.
        String choppedLine(rawLine + 3, 0, length - 3);
        Logger::Log(LogCategory::Info, String::FromFormat("Emissive: [{0}]\n", choppedLine));
      }
      else {
        String choppedLine(rawLine, 0, length);
        Logger::Log(LogCategory::Info, String::FromFormat("Unknown Line: [{0}]\n", choppedLine));
      }
      break;
    }
    break;
    case 'd':
    {
      // Dissolve.
      String choppedLine(rawLine + 2, 0, length - 2);
      Logger::Log(LogCategory::Info, String::FromFormat("Dissolve: [{0}]\n", choppedLine));
    }
    break;
    case 'i':
    {
      // Illumination Mode.
      String choppedLine(rawLine + 6, 0, length - 6);
      Logger::Log(LogCategory::Info, String::FromFormat("Illumination Mode: [{0}]\n", choppedLine));
    }
    break;
    case 'm':
    {
      // Texture map file.
      String choppedLine(rawLine, 0, length);

      if (choppedLine.FindString("map_Kd") != nullptr) {
        String filename(rawLine + 7, 0, length - 7);
        mAlbedoTexture = filename;
      }
      else if (choppedLine.FindString("map_Bump") != nullptr) {
        String filename(rawLine + 9, 0, length - 9);
        Logger::Log(LogCategory::Info, String::FromFormat("Bump map: [{0}]\n", filename));
      }
      else if (choppedLine.FindString("map_Ks") != nullptr) {
        String filename(rawLine + 7, 0, length - 7);
        Logger::Log(LogCategory::Info, String::FromFormat("Specular map: [{0}]\n", filename));
      }
      else {
        String filename(rawLine, 0, length);
        Logger::Log(LogCategory::Info, String::FromFormat("Unknown map: [{0}]\n", filename));
      }
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

void OBJFile::ParseVec3(float fillVec[3], String& line, float fallback) {
  for (int32 i = 0; i < 3; ++i) {
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

void OBJFile::ParseVec4(float fillVec[4], String& line, float fallback) {
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

void OBJFile::ParseMaterial(String& line, const FileString& objFilePath) {
  FileString materialPath(objFilePath);
  materialPath.SetFilename(line);

  MemoryMappedFileReader reader(materialPath);
  if (!reader.IsOpen()) {
    Logger::Log(LogCategory::Info, String::FromFormat("Material file doesn't exist: [{0}]\n", materialPath.GetAbsolutePath()));
    return;
  }

  const char* fileBuffer = reader.GetBuffer();
  size_t fileSize = reader.GetSize();
  size_t lastLineIndex = 0;

  for (size_t offset = 0; offset < fileSize; ++offset) {
    if (fileBuffer[offset] == '\n') {
      size_t lineLength = offset - lastLineIndex;
      ParseMTLLine(fileBuffer + lastLineIndex, lineLength, materialPath);
      lastLineIndex = offset + 1;
    }
  }
}

}
