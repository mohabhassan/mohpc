#pragma once

#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/IFileManager.h>
#include <MOHPC/Files/FileDefs.h>
#include <MOHPC/Common/Log.h>

const MOHPC::fs::path& GetGamePathFromCommandLine();
const std::string& GetNamePassFromCommandLine();
const int GetServerPortFromCommandLine();
MOHPC::AssetManagerPtr AssetLoad(const MOHPC::fs::path& path);
void InitCommon(int argc, const char* argv[]);
