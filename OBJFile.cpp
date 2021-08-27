#include "OBJFile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <string>

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

const std::vector<ZSharp::Vec4<float>>& OBJFile::GetVerts() {
  return mVerts;
}

const std::vector<OBJFace>& OBJFile::GetFaces() {
  return mFaces;
}

void OBJFile::Serialize(FileString& destPath) {
  std::ofstream file(destPath.GetAbsolutePath(), std::ios::out | std::ios::binary | std::ios::trunc);
  if (file.is_open()) {
    // Write length of vertex vector
    std::size_t vertSize = mVerts.size();
    file.write(reinterpret_cast<char*>(&vertSize), sizeof(std::size_t));

    // Write vertex vector
    file.write(reinterpret_cast<char*>(mVerts.data()), vertSize * sizeof(ZSharp::Vec4<float>));

    // Write size of face vector
    std::size_t faceSize = mFaces.size();
    file.write(reinterpret_cast<char*>(&faceSize), sizeof(std::size_t));

    // Write face vector
    file.write(reinterpret_cast<char*>(mFaces.data()), faceSize * sizeof(OBJFace));

    file.flush();
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

void OBJFile::Deserialize(FileString& objFilePath) {
  std::ifstream file(objFilePath.GetAbsolutePath(), std::ios::in | std::ios::binary);
  if (file.is_open()) {
    // Read vert size
    std::size_t vertSize = 0;
    file.read(reinterpret_cast<char*>(&vertSize), sizeof(std::size_t));
    mVerts.resize(vertSize);

    // Read verticies
    file.read(reinterpret_cast<char*>(mVerts.data()), vertSize * sizeof(ZSharp::Vec4<float>));

    // Read face size
    std::size_t faceSize = 0;
    file.read(reinterpret_cast<char*>(&faceSize), sizeof(std::size_t));
    mFaces.resize(faceSize);

    // Read face vector
    file.read(reinterpret_cast<char*>(mFaces.data()), faceSize * sizeof(OBJFace));

    file.close();
  }
}

void OBJFile::ParseLine(std::string& currentLine) {
  const char* rawLine = currentLine.c_str();
  
  switch (rawLine[0]) {
    case 'v':
      if (rawLine[1] == 'n') {
        // Vertex Normals.
        ZSharp::Vec3<float> vertex;
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
        ZSharp::Vec3<float> vertex;
        std::string choppedLine(rawLine + 3);
        ParseVec3(vertex, choppedLine, 0.0f);
        mUVCoords.push_back(vertex);
      }
      else {
        // Vertex Data.
        ZSharp::Vec4<float> vertex;
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

void OBJFile::ParseVec3(ZSharp::Vec3<float>& fillVec, std::string& line, float fallback) {
  ZSharp::Vec4<float> sacrifice;
  ParseVec4(sacrifice, line, fallback);
  std::memcpy(*fillVec, *sacrifice, sizeof(float) * 3);
}

void OBJFile::ParseVec4(ZSharp::Vec4<float>& fillVec, std::string& line, float fallback) {
  std::size_t nextPos = 0;

  for (std::int32_t i = 0; i < 4; ++i) {
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
  std::size_t nextPos = 0;
  for (std::int32_t i = 0; i < 3; ++i) {
    if (!line.empty()) {
      std::uint64_t vertexIndex = std::stoull(line, &nextPos);
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
      std::uint64_t textureIndex = std::stoull(line, &nextPos);
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
      std::uint64_t normalIndex = std::stoull(line, &nextPos);
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
