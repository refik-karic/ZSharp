#include "OBJFile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <string>

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
  std::ofstream file(destPath.GetAbsolutePath(), std::ios::out | std::ios::binary | std::ios::trunc);
  if (file.is_open()) {
    // Write length of vertex vector
    size_t vertSize = mVerts.Size();
    file.write(reinterpret_cast<char*>(&vertSize), sizeof(size_t));

    // Write vertex vector
    for (Vec4& vector : mVerts) {
      file.write(reinterpret_cast<char*>(&vector), sizeof(Vec4));
    }

    // Write length of the normal vector
    size_t normalSize = mNormals.Size();
    file.write(reinterpret_cast<char*>(&normalSize), sizeof(size_t));

    // Write normal vector
    for (Vec3& normal : mNormals) {
      file.write(reinterpret_cast<char*>(&normal), sizeof(Vec3));
    }

    // Write length of the uv vector
    size_t uvSize = mUVCoords.Size();
    file.write(reinterpret_cast<char*>(&uvSize), sizeof(size_t));

    // Write uv vector
    for (Vec3& uv : mUVCoords) {
      file.write(reinterpret_cast<char*>(&uv), sizeof(Vec3));
    }

    // Write size of face vector
    size_t faceSize = mFaces.Size();
    file.write(reinterpret_cast<char*>(&faceSize), sizeof(size_t));

    // Write face vector
    for (OBJFace& face : mFaces) {
      file.write(reinterpret_cast<char*>(&face), sizeof(OBJFace));
    }

    file.flush();
    file.close();
  }
}

void OBJFile::Deserialize(FileString& objFilePath) {
  std::ifstream file(objFilePath.GetAbsolutePath(), std::ios::in | std::ios::binary);
  if (file.is_open()) {
    // Read vert size
    size_t vertSize = 0;
    file.read(reinterpret_cast<char*>(&vertSize), sizeof(size_t));
    mVerts.Resize(vertSize);

    // Read verticies
    for (size_t i = 0; i < vertSize; ++i) {
      Vec4 vector;
      file.read(reinterpret_cast<char*>(&vector), sizeof(Vec4));
      mVerts[i] = vector;
    }

    // Read normal size
    size_t normalSize = 0;
    file.read(reinterpret_cast<char*>(&normalSize), sizeof(size_t));
    mNormals.Resize(normalSize);

    // Read normals
    for (size_t i = 0; i < normalSize; ++i) {
      Vec3 normal;
      file.read(reinterpret_cast<char*>(&normal), sizeof(Vec3));
      mNormals[i] = normal;
    }

    // Read uv size
    size_t uvSize = 0;
    file.read(reinterpret_cast<char*>(&uvSize), sizeof(size_t));
    mUVCoords.Resize(uvSize);

    // Read uvs
    for (size_t i = 0; i < uvSize; ++i) {
      Vec3 uv;
      file.read(reinterpret_cast<char*>(&uv), sizeof(Vec3));
      mUVCoords[i] = uv;
    }

    // Read face size
    size_t faceSize = 0;
    file.read(reinterpret_cast<char*>(&faceSize), sizeof(size_t));
    mFaces.Resize(faceSize);

    // Read face vector
    for (size_t i = 0; i < faceSize; ++i) {
      OBJFace face;
      file.read(reinterpret_cast<char*>(&face), sizeof(OBJFace));
      mFaces[i] = face;
    }

    file.close();
  }
}

void OBJFile::SetVerboseParse(bool state) {
  mVerboseParse = state;
}

void OBJFile::ParseRaw(FileString& objFilePath) {
  std::ifstream file(objFilePath.GetAbsolutePath());
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      if (line.length() > 2) {
        ParseLine(line);
      }
    }
    file.close();
  }
}

void OBJFile::ParseLine(std::string& currentLine) {
  const char* rawLine = currentLine.c_str();

  switch (rawLine[0]) {
  case 'v':
    if (rawLine[1] == 'n') {
      // Vertex Normals.
      Vec3 vertex;
      std::string choppedLine(rawLine + 3);
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
      std::string choppedLine(rawLine + 3);
      ParseVec3(vertex, choppedLine, 0.0f);
      mUVCoords.PushBack(vertex);
    }
    else {
      // Vertex Data.
      Vec4 vertex;
      std::string choppedLine(rawLine + 2);
      ParseVec4(vertex, choppedLine, 1.0f);
      mVerts.PushBack(vertex);
    }
    break;
  case 'f':
    // Vertex face.
  {
    OBJFace face;
    std::string choppedLine(rawLine + 2);
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

void OBJFile::ParseVec3(Vec3& fillVec, std::string& line, float fallback) {
  Vec4 sacrifice;
  ParseVec4(sacrifice, line, fallback);
  memcpy(*fillVec, *sacrifice, sizeof(float) * 3);
}

void OBJFile::ParseVec4(Vec4& fillVec, std::string& line, float fallback) {
  size_t nextPos = 0;

  for (int32_t i = 0; i < 4; ++i) {
    if (!line.empty()) {
      fillVec[i] = std::stof(line, &nextPos);
    }
    else {
      fillVec[i] = fallback;
      return;
    }

    line = line.substr(nextPos);
  }
}

void OBJFile::ParseFace(OBJFace& fillFace, std::string& line) {
  size_t nextPos = 0;
  for (int32_t i = 0; i < 3; ++i) {
    if (!line.empty()) {
      uint64_t vertexIndex = std::stoull(line, &nextPos);
      ++nextPos;
      fillFace.triangleFace[i].vertexIndex = vertexIndex;

      if (nextPos >= line.length()) {
        line.clear();
      }
      else {
        line = line.substr(nextPos);
      }
    }
    else {
      return;
    }

    if (!line.empty()) {
      uint64_t textureIndex = std::stoull(line, &nextPos);
      ++nextPos;
      fillFace.triangleFace[i].uvIndex = textureIndex;

      if (nextPos >= line.length()) {
        line.clear();
      }
      else {
        line = line.substr(nextPos);
      }
    }
    else {
      return;
    }

    if (!line.empty()) {
      uint64_t normalIndex = std::stoull(line, &nextPos);
      ++nextPos;
      fillFace.triangleFace[i].normalIndex = normalIndex;

      if (nextPos >= line.length()) {
        line.clear();
      }
      else {
        line = line.substr(nextPos);
      }
    }
    else {
      return;
    }
  }
}
}
