#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <windows.h>

#include "config/Config.h"
#include "core/Task.h"
#include "llm/LlmClient.h"
#include "storage/JsonStorage.h"

class TaskService {
 public:
  TaskService(AppConfig& config, storage::JsonStorage& storage, llm::LlmClient& llm, HWND notifyHwnd);

  const std::vector<Task>& Tasks() const { return tasks_; }

  void Reload();
  bool Persist();

  std::uint64_t AddTask(const std::string& textUtf8, std::int64_t deadlineUtcMs, int checkAfterMinutes);
  bool MarkDone(std::uint64_t id);
  bool Remove(std::uint64_t id);
  void OnLlmSuccess(std::uint64_t id);
  void OnLlmError(std::uint64_t id);

  void OnTimerTick();

 private:
  static std::int64_t NowUtcMs();
  Task* Find(std::uint64_t id);

  AppConfig& config_;
  storage::JsonStorage& storage_;
  llm::LlmClient& llm_;
  HWND hwnd_;
  std::vector<Task> tasks_;
  std::uint64_t idCounter_ = 1;
};
