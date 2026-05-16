#pragma once

#include <string>

struct AppConfig {
  int timerIntervalSec = 5;
  int llmHttpTimeoutSec = 60;
  int llmErrorLogIntervalSec = 30;
  int defaultCheckAfterMinutes = 15;
  std::string ollamaHost = "127.0.0.1";
  int ollamaPort = 11434;
  std::string ollamaModel = "llama3.2";
  std::string redisHost = "127.0.0.1";
  int redisPort = 6379;
  std::string redisKeyPrefix = "taskdevourer:dialog:";
};

namespace config {

bool Load(AppConfig& out, const std::wstring& exeDir);
std::wstring ConfigPath(const std::wstring& exeDir);

}  // namespace config
