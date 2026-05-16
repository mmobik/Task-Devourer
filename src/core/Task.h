#pragma once

#include <cstdint>
#include <string>

enum class TaskStatus { NotStarted, Completed, Overdue };

struct Task {
  std::uint64_t id = 0;
  std::string text;
  TaskStatus status = TaskStatus::NotStarted;
  std::int64_t deadlineUtcMs = 0;
  int checkAfterMinutes = 15;
  bool llmAskedForOverdue = false;
};
