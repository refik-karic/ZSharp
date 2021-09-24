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

const std::vector<ZSharp::Vec4>& OBJFile::GetVerts() {
  return mVerts;
}

const std::vector<OBJFace>& OBJFile::GetFaces() {
  return mFaces;
}

void OBJFile::Serialize(FileString& destPath) {
  std::ofstream file(destPath.GetAbsolutePath(), std::ios::out | std::ios::binary | std::ios::trunc);
  if (file.is_open()) {
    // Write length of vertex vector
    size_t vertSize = mVerts.size();
    file.write(reinterpret_cast<char*>(&vertSize), sizeof(size_t));

    // Write vertex vector
    for (Vec4& vector : mVerts) {
      file.write(reinterpret_cast<char*>(&vector), sizeof(ZSharp::Vec4));
    }

    // Write size of face vector
    size_t faceSize = mFaces.size();
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
    mVerts.resize(vertSize);

    // Read verticies
    for (size_t i = 0; i < vertSize; ++i) {
      Vec4 vector;
      file.read(reinterpret_cast<char*>(&vector), sizeof(ZSharp::Vec4));
      mVerts[i] = vector;
    }

    // Read face size
    size_t faceSize = 0;
    file.read(reinterpret_cast<char*>(&faceSize), sizeof(size_t));
    mFaces.resize(faceSize);

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
      ZSharp::Vec3 vertex;
      std::string choppedLine(rawLine + 3);
      ParseVec3(vertex, choppedLine, 0.0f);
      mNormals.push_back(vertex);
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
      ZSharp::Vec3 vertex;
      std::string choppedLine(rawLine + 3);
      ParseVec3(vertex, choppedLine, 0.0f);
      mUVCoords.push_back(vertex);
    }
    else {
      // Vertex Data.
      ZSharp::Vec4 vertex;
      std::string choppedLine(rawLine + 2);
      ParseVec4(vertex, choppedLine, 1.0f);
      mVerts.push_back(vertex);
    }
    break;
  case 'f':
    // Vertex face.
  {
    OBJFace face;
    std::string choppedLine(rawLine + 2);
    ParseFace(face, choppedLine);
    mFaces.push_back(face);
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

void OBJFile::ParseVec3(ZSharp::Vec3& fillVec, std::string& line, float fallback) {
  ZSharp::Vec4 sacrifice;
  ParseVec4(sacrifice, line, fallback);
  std::memcpy(*fillVec, *sacrifice, sizeof(float) * 3);
}

void OBJFile::ParseVec4(ZSharp::Vec4& fillVec, std::string& line, float fallback) {
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
