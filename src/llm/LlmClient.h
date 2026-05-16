#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <windows.h>

#include "config/Config.h"

struct LlmMessagePayload {
  std::uint64_t taskId = 0;
  std::wstring userVisibleText;
  std::string promptUtf8;
  std::string responseUtf8;
};

namespace llm {

class LlmClient {
 public:
  explicit LlmClient(const AppConfig& config);

  bool IsInFlight() const { return inFlight_.load(); }

  void AskOverdueHelp(HWND hwnd, std::uint64_t taskId, const std::string& taskTextUtf8);

 private:
  AppConfig config_;
  std::atomic<bool> inFlight_{false};
};

}  // namespace llm
