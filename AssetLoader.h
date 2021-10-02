#pragma once

#include "FileString.h"
#include "Model.h"

namespace ZSharp {

void LoadModelJSON(const char* fileName, Model& model);

void LoadModelOBJ(FileString& fileName, Model& model);
}
