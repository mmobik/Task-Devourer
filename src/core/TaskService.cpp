#include "core/TaskService.h"

#include <chrono>

#include "app/AppMessages.h"
#include "log/Logger.h"

TaskService::TaskService(AppConfig& config, storage::JsonStorage& storage, llm::LlmClient& llm,
                         HWND notifyHwnd)
    : config_(config), storage_(storage), llm_(llm), hwnd_(notifyHwnd) {
  Reload();
}

void TaskService::Reload() {
  std::vector<std::string> titles;
  storage_.Load(titles);
  tasks_.clear();
  const std::int64_t now = NowUtcMs();
  const std::int64_t defaultDeadline = now + 24LL * 60 * 60 * 1000;
  for (const auto& text : titles) {
    Task t;
    t.id = idCounter_++;
    t.text = text;
    t.deadlineUtcMs = defaultDeadline;
    t.checkAfterMinutes = config_.defaultCheckAfterMinutes;
    t.status = TaskStatus::NotStarted;
    tasks_.push_back(std::move(t));
  }
}

bool TaskService::Persist() { return storage_.Save(tasks_); }

std::int64_t TaskService::NowUtcMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

Task* TaskService::Find(const std::uint64_t id) {
  for (auto& t : tasks_) {
    if (t.id == id) return &t;
  }
  return nullptr;
}

std::uint64_t TaskService::AddTask(const std::string& textUtf8, const std::int64_t deadlineUtcMs,
                                   const int checkAfterMinutes) {
  Task t;
  t.id = idCounter_++;
  t.text = textUtf8;
  t.deadlineUtcMs = deadlineUtcMs;
  t.checkAfterMinutes = checkAfterMinutes > 0 ? checkAfterMinutes : config_.defaultCheckAfterMinutes;
  t.status = TaskStatus::NotStarted;
  tasks_.push_back(std::move(t));
  Persist();
  PostMessageW(hwnd_, WM_APP_REFRESH_LIST, 0, 0);
  return tasks_.back().id;
}

bool TaskService::MarkDone(const std::uint64_t id) {
  Task* t = Find(id);
  if (!t) return false;
  t->status = TaskStatus::Completed;
  t->llmAskedForOverdue = false;
  Persist();
  PostMessageW(hwnd_, WM_APP_REFRESH_LIST, 0, 0);
  return true;
}

bool TaskService::Remove(const std::uint64_t id) {
  for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
    if (it->id == id) {
      tasks_.erase(it);
      Persist();
      PostMessageW(hwnd_, WM_APP_REFRESH_LIST, 0, 0);
      return true;
    }
  }
  return false;
}

void TaskService::OnLlmSuccess(const std::uint64_t id) {
  if (Task* t = Find(id)) {
    t->llmAskedForOverdue = true;
  }
}

void TaskService::OnLlmError(const std::uint64_t id) {
  if (Task* t = Find(id)) {
    t->llmAskedForOverdue = false;
  }
}

void TaskService::OnTimerTick() {
  const std::int64_t now = NowUtcMs();
  bool changed = false;

  for (auto& t : tasks_) {
    if (t.status == TaskStatus::Completed) continue;

    if (now > t.deadlineUtcMs && t.status != TaskStatus::Overdue) {
      t.status = TaskStatus::Overdue;
      changed = true;
    }

    if (t.status != TaskStatus::Overdue) continue;

    const std::int64_t graceMs = static_cast<std::int64_t>(t.checkAfterMinutes) * 60 * 1000;
    const bool pastGrace = now > (t.deadlineUtcMs + graceMs);
    if (pastGrace && !t.llmAskedForOverdue && !llm_.IsInFlight()) {
      llm_.AskOverdueHelp(hwnd_, t.id, t.text);
    }
  }

  if (changed) {
    PostMessageW(hwnd_, WM_APP_REFRESH_LIST, 0, 0);
  }
}
