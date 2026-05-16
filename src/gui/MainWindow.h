#pragma once

#include <memory>
#include <vector>
#include <windows.h>

#include "config/Config.h"
#include "core/TaskService.h"
#include "llm/LlmClient.h"
#include "redis/DialogLogger.h"
#include "storage/JsonStorage.h"
#include "timer/TimerService.h"

enum ControlId : int {
  IDC_TASK_LIST = 1001,
  IDC_TITLE_EDIT,
  IDC_DEADLINE_PICKER,
  IDC_CHECK_MINUTES_EDIT,
  IDC_BTN_ADD,
  IDC_BTN_DONE,
  IDC_BTN_DELETE,
  IDC_BTN_SAVE,
};

class MainWindow {
 public:
  bool Create(HINSTANCE instance, int showCmd);

 private:
  static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  void OnCreate();
  void RefreshList();
  std::uint64_t SelectedTaskId() const;
  std::int64_t ReadDeadlineUtcMs() const;
  int ReadCheckMinutes() const;

  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
  HWND list_ = nullptr;
  HWND titleEdit_ = nullptr;
  HWND deadlinePicker_ = nullptr;
  HWND checkMinutesEdit_ = nullptr;

  AppConfig config_{};
  std::unique_ptr<storage::JsonStorage> storage_;
  std::unique_ptr<llm::LlmClient> llm_;
  std::unique_ptr<redislog::DialogLogger> redis_;
  std::unique_ptr<TaskService> tasks_;
  timerx::TimerService timer_;

  std::vector<std::wstring> listLines_;
  std::int64_t lastLlmErrorLogMs_ = 0;
};
