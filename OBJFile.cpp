#include "OBJFile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma warning(disable : 4996)

size_t ReadLine(char** data, size_t* length, FILE* file)
{
  if (data == nullptr || *data == nullptr || length == nullptr || file == nullptr) {
    return 0;
  }

  bool keepReading = true;
  size_t numRead = 0;
  for (size_t i = 0; keepReading; ++i) {
    if (i == (*length - 1)) {
      size_t newLength = *length + 512;
      char* resizedBuf = static_cast<char*>(realloc(*data, newLength));
      if (resizedBuf == nullptr) {
        return 0;
      }

      *data = resizedBuf;
      *length = newLength;
    }

    char nextChar = static_cast<char>(fgetc(file));
    if (nextChar == EOF) {
      (*data)[i] = NULL;
      keepReading = false;
    }
    else {
      (*data)[i] = nextChar;
      numRead++;
    }

    if ((*data)[i] == '\n') {
      (*data)[i + 1] = NULL;
      keepReading = false;
    }
  }

  return numRead;
}

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
  FILE* file = fopen(destPath.GetAbsolutePath().Str(), "wb");
  if (file != nullptr) {
    // Write length of vertex vector
    size_t vertSize = mVerts.Size();
    fwrite(reinterpret_cast<char*>(&vertSize), sizeof(size_t), 1, file);

    // Write vertex vector
    for (Vec4& vector : mVerts) {
      fwrite(reinterpret_cast<char*>(&vector), sizeof(Vec4), 1, file);
    }

    // Write length of the normal vector
    size_t normalSize = mNormals.Size();
    fwrite(reinterpret_cast<char*>(&normalSize), sizeof(size_t), 1, file);

    // Write normal vector
    for (Vec3& normal : mNormals) {
      fwrite(reinterpret_cast<char*>(&normal), sizeof(Vec3), 1, file);
    }

    // Write length of the uv vector
    size_t uvSize = mUVCoords.Size();
    fwrite(reinterpret_cast<char*>(&uvSize), sizeof(size_t), 1, file);

    // Write uv vector
    for (Vec3& uv : mUVCoords) {
      fwrite(reinterpret_cast<char*>(&uv), sizeof(Vec3), 1, file);
    }

    // Write size of face vector
    size_t faceSize = mFaces.Size();
    fwrite(reinterpret_cast<char*>(&faceSize), sizeof(size_t), 1, file);

    // Write face vector
    for (OBJFace& face : mFaces) {
      fwrite(reinterpret_cast<char*>(&face), sizeof(OBJFace), 1, file);
    }

    fflush(file);
    fclose(file);
  }
}

void OBJFile::Deserialize(FileString& objFilePath) {
  const char* fileName = objFilePath.GetAbsolutePath().Str();
  FILE* file = fopen(fileName, "rb");
  if (file != nullptr) {
    // Read vert size
    size_t vertSize = 0;
    fread(reinterpret_cast<char*>(&vertSize), sizeof(size_t), 1, file);
    mVerts.Resize(vertSize);

    // Read verticies
    for (size_t i = 0; i < vertSize; ++i) {
      Vec4 vector;
      fread(reinterpret_cast<char*>(&vector), sizeof(Vec4), 1, file);
      mVerts[i] = vector;
    }

    // Read normal size
    size_t normalSize = 0;
    fread(reinterpret_cast<char*>(&normalSize), sizeof(size_t), 1, file);
    mNormals.Resize(normalSize);

    // Read normals
    for (size_t i = 0; i < normalSize; ++i) {
      Vec3 normal;
      fread(reinterpret_cast<char*>(&normal), sizeof(Vec3), 1, file);
      mNormals[i] = normal;
    }

    // Read uv size
    size_t uvSize = 0;
    fread(reinterpret_cast<char*>(&uvSize), sizeof(size_t), 1, file);
    mUVCoords.Resize(uvSize);

    // Read uvs
    for (size_t i = 0; i < uvSize; ++i) {
      Vec3 uv;
      fread(reinterpret_cast<char*>(&uv), sizeof(Vec3), 1, file);
      mUVCoords[i] = uv;
    }

    // Read face size
    size_t faceSize = 0;
    fread(reinterpret_cast<char*>(&faceSize), sizeof(size_t), 1, file);
    mFaces.Resize(faceSize);

    // Read face vector
    for (size_t i = 0; i < faceSize; ++i) {
      OBJFace face;
      fread(reinterpret_cast<char*>(&face), sizeof(OBJFace), 1, file);
      mFaces[i] = face;
    }

    fclose(file);
  }
}

void OBJFile::SetVerboseParse(bool state) {
  mVerboseParse = state;
}

void OBJFile::ParseRaw(FileString& objFilePath) {
  FILE* file = fopen(objFilePath.GetAbsolutePath().Str(), "r");
  if (file != nullptr) {
    size_t bufferSize = 512;
    char* buffer = static_cast<char*>(malloc(bufferSize));
    for (size_t read = ReadLine(&buffer, &bufferSize, file); read > 0; read = ReadLine(&buffer, &bufferSize, file)) {
      if (read > 2) {
        buffer[read - 1] = NULL;

        ParseLine(buffer);
        char* resizedBuf = static_cast<char*>(realloc(buffer, bufferSize));
        if (resizedBuf == nullptr) {
          break;
        }

        buffer = resizedBuf;
        memset(buffer, 0, bufferSize);
      }
    }

    free(buffer);
    fclose(file);
  }
}

void OBJFile::ParseLine(const char* currentLine) {
  const char* rawLine = currentLine;

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
      if (mVerboseParse) {
        printf("Vertex Parameters: [%s]\n", rawLine);
      }
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
    if (mVerboseParse) {
      printf("Line: [%s]\n", rawLine);
    }
    break;
  default:
    if (mVerboseParse) {
      printf("[%s] Unknown Line\n", rawLine);
    }
    break;
  }
}

void OBJFile::ParseVec3(Vec3& fillVec, String& line, float fallback) {
  Vec4 sacrifice;
  ParseVec4(sacrifice, line, fallback);
  memcpy(*fillVec, *sacrifice, sizeof(float) * 3);
}

void OBJFile::ParseVec4(Vec4& fillVec, String& line, float fallback) {
  char* nextPos = NULL;
  
  for (int32_t i = 0; i < 4; ++i) {
    if (!line.IsEmpty()) {
      fillVec[i] = strtof(line.Str(), &nextPos);
    }
    else {
      fillVec[i] = fallback;
      return;
    }

    line = line.SubStr(nextPos - line.Str(), line.GetSize());
  }
}

void OBJFile::ParseFace(OBJFace& fillFace, String& line) {
  char* nextPos = NULL;
  for (int32_t i = 0; i < 3; ++i) {
    if (!line.IsEmpty()) {
      uint64_t vertexIndex = strtoull(line.Str(), &nextPos, 10);
      ++nextPos;
      fillFace.triangleFace[i].vertexIndex = vertexIndex;

      if (nextPos == NULL) {
        line.Clear();
      }
      else {
        line = line.SubStr(nextPos - line.Str(), line.GetSize());
      }
    }
    else {
      return;
    }

    if (!line.IsEmpty()) {
      uint64_t textureIndex = strtoull(line.Str(), &nextPos, 10);
      ++nextPos;
      fillFace.triangleFace[i].uvIndex = textureIndex;

      if (nextPos == NULL) {
        line.Clear();
      }
      else {
        line = line.SubStr(nextPos - line.Str(), line.GetSize());
      }
    }
    else {
      return;
    }

    if (!line.IsEmpty()) {
      uint64_t normalIndex = strtoull(line.Str(), &nextPos, 10);
      ++nextPos;
      fillFace.triangleFace[i].normalIndex = normalIndex;

      if (nextPos == NULL) {
        line.Clear();
      }
      else {
        line = line.SubStr(nextPos - line.Str(), line.GetSize());
      }
    }
    else {
      return;
    }
  }
}
}

#pragma warning(default : 4996)
