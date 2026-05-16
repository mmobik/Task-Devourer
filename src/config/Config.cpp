#include "config/Config.h"

#include <windows.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "log/Logger.h"

namespace config {

namespace {

void ApplyDefaults(AppConfig& cfg) {
  AppConfig defaults;
  if (cfg.timerIntervalSec <= 0) cfg.timerIntervalSec = defaults.timerIntervalSec;
  if (cfg.llmHttpTimeoutSec <= 0) cfg.llmHttpTimeoutSec = defaults.llmHttpTimeoutSec;
  if (cfg.llmErrorLogIntervalSec <= 0) cfg.llmErrorLogIntervalSec = defaults.llmErrorLogIntervalSec;
  if (cfg.defaultCheckAfterMinutes <= 0)
    cfg.defaultCheckAfterMinutes = defaults.defaultCheckAfterMinutes;
  if (cfg.ollamaHost.empty()) cfg.ollamaHost = defaults.ollamaHost;
  if (cfg.ollamaPort <= 0) cfg.ollamaPort = defaults.ollamaPort;
  if (cfg.ollamaModel.empty()) cfg.ollamaModel = defaults.ollamaModel;
  if (cfg.redisHost.empty()) cfg.redisHost = defaults.redisHost;
  if (cfg.redisPort <= 0) cfg.redisPort = defaults.redisPort;
  if (cfg.redisKeyPrefix.empty()) cfg.redisKeyPrefix = defaults.redisKeyPrefix;
}

bool CopyExampleIfNeeded(const std::wstring& exeDir) {
  const std::wstring cfgPath = exeDir + L"config.json";
  const std::wstring examplePath = exeDir + L"config.json.example";
  if (GetFileAttributesW(cfgPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
    return true;
  }
  if (GetFileAttributesW(examplePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return CopyFileW(examplePath.c_str(), cfgPath.c_str(), FALSE) != 0;
}

}  // namespace

std::wstring ConfigPath(const std::wstring& exeDir) { return exeDir + L"config.json"; }

bool Load(AppConfig& out, const std::wstring& exeDir) {
  CopyExampleIfNeeded(exeDir);
  ApplyDefaults(out);

  const std::wstring path = ConfigPath(exeDir);
  std::ifstream in(path);
  if (!in) {
    logx::Logger::Instance().Info("config.json missing; using defaults");
    return false;
  }

  try {
    nlohmann::json j;
    in >> j;
    if (j.contains("timerIntervalSec")) out.timerIntervalSec = j["timerIntervalSec"].get<int>();
    if (j.contains("llmHttpTimeoutSec")) out.llmHttpTimeoutSec = j["llmHttpTimeoutSec"].get<int>();
    if (j.contains("llmErrorLogIntervalSec"))
      out.llmErrorLogIntervalSec = j["llmErrorLogIntervalSec"].get<int>();
    if (j.contains("defaultCheckAfterMinutes"))
      out.defaultCheckAfterMinutes = j["defaultCheckAfterMinutes"].get<int>();
    if (j.contains("checkAfterMinutes"))
      out.defaultCheckAfterMinutes = j["checkAfterMinutes"].get<int>();
    if (j.contains("ollamaHost")) out.ollamaHost = j["ollamaHost"].get<std::string>();
    if (j.contains("ollamaPort")) out.ollamaPort = j["ollamaPort"].get<int>();
    if (j.contains("ollamaModel")) out.ollamaModel = j["ollamaModel"].get<std::string>();
    if (j.contains("redisHost")) out.redisHost = j["redisHost"].get<std::string>();
    if (j.contains("redisPort")) out.redisPort = j["redisPort"].get<int>();
    if (j.contains("redisKeyPrefix")) out.redisKeyPrefix = j["redisKeyPrefix"].get<std::string>();
    ApplyDefaults(out);
    return true;
  } catch (const std::exception& ex) {
    logx::Logger::Instance().Error(std::string("config parse error: ") + ex.what());
    ApplyDefaults(out);
    return false;
  }
}

}  // namespace config
