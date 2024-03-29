#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "Array.h"
#include "Asset.h"
#include "Serializer.h"

namespace ZSharp {

bool GenerateBundle(const FileString& filename, Array<Asset>& assets, MemorySerializer& data);

void SerializeOBJFile(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory);

void SerializeTexturePNG(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory);

void SerializeTextureJPG(const FileString& filename, Array<Asset>& bundleAssets, MemorySerializer& bundleMemory);

}
