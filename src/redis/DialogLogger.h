#pragma once

#include <cstdint>
#include <string>

#include "config/Config.h"

namespace redislog {

class DialogLogger {
 public:
  explicit DialogLogger(const AppConfig& config);

  void Append(std::uint64_t taskId, const std::string& promptUtf8, const std::string& responseUtf8);

 private:
  void EnsureConnected();

  AppConfig config_;
  void* context_ = nullptr;
};

}  // namespace redislog
