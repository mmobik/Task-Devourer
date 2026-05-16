#include "gui/MainWindow.h"

#include <commctrl.h>
#include <sstream>

#include "app/AppMessages.h"
#include "llm/LlmClient.h"
#include "log/Logger.h"
#include "util/Paths.h"
#include "util/StringUtil.h"

namespace {

constexpr wchar_t kClassName[] = L"TaskDevourerMainWindow";
constexpr int kWindowWidth = 720;
constexpr int kWindowHeight = 520;

std::int64_t FileTimeToUtcMs(const FILETIME& ft) {
  ULARGE_INTEGER uli;
  uli.LowPart = ft.dwLowDateTime;
  uli.HighPart = ft.dwHighDateTime;
  constexpr ULONGLONG kEpochDiff = 116444736000000000ULL;
  if (uli.QuadPart < kEpochDiff) {
    return 0;
  }
  return static_cast<std::int64_t>((uli.QuadPart - kEpochDiff) / 10000ULL);
}

std::int64_t NowUtcMs() {
  FILETIME ft{};
  GetSystemTimeAsFileTime(&ft);
  return FileTimeToUtcMs(ft);
}

const wchar_t* StatusPrefix(TaskStatus s) {
  switch (s) {
    case TaskStatus::Completed:
      return L"[x] ";
    case TaskStatus::Overdue:
      return L"[!] ";
    default:
      return L"[ ] ";
  }
}

}  // namespace

bool MainWindow::Create(HINSTANCE instance, const int showCmd) {
  instance_ = instance;
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = &MainWindow::StaticWndProc;
  wc.hInstance = instance;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wc.lpszClassName = kClassName;
  RegisterClassExW(&wc);

  hwnd_ = CreateWindowExW(0, kClassName, L"Task Devourer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                          CW_USEDEFAULT, kWindowWidth, kWindowHeight, nullptr, nullptr, instance,
                          this);
  if (!hwnd_) {
    return false;
  }
  ShowWindow(hwnd_, showCmd);
  UpdateWindow(hwnd_);
  return true;
}

LRESULT CALLBACK MainWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  MainWindow* self = nullptr;
  if (msg == WM_NCCREATE) {
    auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
    self = static_cast<MainWindow*>(cs->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    self->hwnd_ = hwnd;
  } else {
    self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  }
  if (self) {
    return self->WndProc(hwnd, msg, wParam, lParam);
  }
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void MainWindow::OnCreate() {
  const std::wstring exeDir = util::GetExeDirectory();
  logx::Logger::Instance().SetLogPath(exeDir + L"app.log");
  config::Load(config_, exeDir);

  storage_ = std::make_unique<storage::JsonStorage>(exeDir + L"tasks.json");
  llm_ = std::make_unique<llm::LlmClient>(config_);
  redis_ = std::make_unique<redislog::DialogLogger>(config_);
  tasks_ = std::make_unique<TaskService>(config_, *storage_, *llm_, hwnd_);

  const int margin = 10;
  int y = margin;
  const int editW = kWindowWidth - 2 * margin - 30;

  CreateWindowExW(0, L"STATIC", L"Task", WS_CHILD | WS_VISIBLE, margin, y, 50, 20, hwnd_, nullptr,
                  instance_, nullptr);
  titleEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                               margin + 55, y - 2, editW - 55, 24, hwnd_,
                               reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_TITLE_EDIT)),
                               instance_, nullptr);

  y += 32;
  CreateWindowExW(0, L"STATIC", L"Deadline", WS_CHILD | WS_VISIBLE, margin, y, 70, 20, hwnd_, nullptr,
                  instance_, nullptr);
  deadlinePicker_ = CreateWindowExW(
      0, DATETIMPICK_CLASSW, L"", WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT, margin + 75, y - 2,
      200, 24, hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_DEADLINE_PICKER)), instance_,
      nullptr);

  CreateWindowExW(0, L"STATIC", L"Check min", WS_CHILD | WS_VISIBLE, margin + 290, y, 70, 20, hwnd_,
                  nullptr, instance_, nullptr);
  wchar_t buf[16];
  swprintf_s(buf, L"%d", config_.defaultCheckAfterMinutes);
  checkMinutesEdit_ = CreateWindowExW(
      WS_EX_CLIENTEDGE, L"EDIT", buf, WS_CHILD | WS_VISIBLE | ES_NUMBER, margin + 365, y - 2, 50,
      24, hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHECK_MINUTES_EDIT)), instance_,
      nullptr);

  y += 36;
  const int btnW = 90;
  int x = margin;
  auto makeBtn = [&](const wchar_t* label, const ControlId id) {
    CreateWindowExW(0, L"BUTTON", label, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, btnW, 28,
                    hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance_, nullptr);
    x += btnW + 8;
  };
  makeBtn(L"Add", IDC_BTN_ADD);
  makeBtn(L"Done", IDC_BTN_DONE);
  makeBtn(L"Delete", IDC_BTN_DELETE);
  makeBtn(L"Save", IDC_BTN_SAVE);

  y += 36;
  list_ = CreateWindowExW(
      WS_EX_CLIENTEDGE, L"LISTBOX", L"",
      WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
      margin, y, editW, kWindowHeight - y - 40, hwnd_,
      reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_TASK_LIST)), instance_, nullptr);

  timer_.Start(hwnd_, TIMER_ID_MAIN, static_cast<UINT>(config_.timerIntervalSec * 1000));
  RefreshList();
  logx::Logger::Instance().Info("Task Devourer started");
}

void MainWindow::RefreshList() {
  SendMessageW(list_, LB_RESETCONTENT, 0, 0);
  listLines_.clear();
  for (const auto& t : tasks_->Tasks()) {
    std::wstringstream line;
    line << StatusPrefix(t.status) << util::Utf8ToWide(t.text) << L" (#" << t.id << L")";
    listLines_.push_back(line.str());
    const int idx = static_cast<int>(listLines_.size()) - 1;
    SendMessageW(list_, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(listLines_.back().c_str()));
    SendMessageW(list_, LB_SETITEMDATA, idx, static_cast<LPARAM>(t.id));
  }
}

std::uint64_t MainWindow::SelectedTaskId() const {
  const int sel = static_cast<int>(SendMessageW(list_, LB_GETCURSEL, 0, 0));
  if (sel == LB_ERR) {
    return 0;
  }
  return static_cast<std::uint64_t>(SendMessageW(list_, LB_GETITEMDATA, sel, 0));
}

std::int64_t MainWindow::ReadDeadlineUtcMs() const {
  SYSTEMTIME st{};
  if (DateTime_GetSystemtime(deadlinePicker_, &st) != GDT_VALID) {
    return NowUtcMs();
  }
  FILETIME ftLocal{};
  SystemTimeToFileTime(&st, &ftLocal);
  FILETIME ftUtc{};
  LocalFileTimeToFileTime(&ftLocal, &ftUtc);
  return FileTimeToUtcMs(ftUtc);
}

int MainWindow::ReadCheckMinutes() const {
  wchar_t buf[32]{};
  GetWindowTextW(checkMinutesEdit_, buf, 32);
  const int v = _wtoi(buf);
  return v > 0 ? v : config_.defaultCheckAfterMinutes;
}

LRESULT MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CREATE:
      OnCreate();
      return 0;
    case WM_COMMAND: {
      const int id = LOWORD(wParam);
      if (id == IDC_BTN_ADD) {
        wchar_t title[512]{};
        GetWindowTextW(titleEdit_, title, 512);
        const std::string titleUtf8 = util::WideToUtf8(title);
        if (!titleUtf8.empty()) {
          tasks_->AddTask(titleUtf8, ReadDeadlineUtcMs(), ReadCheckMinutes());
          SetWindowTextW(titleEdit_, L"");
        }
        return 0;
      }
      if (id == IDC_BTN_DONE) {
        const auto tid = SelectedTaskId();
        if (tid != 0) tasks_->MarkDone(tid);
        return 0;
      }
      if (id == IDC_BTN_DELETE) {
        const auto tid = SelectedTaskId();
        if (tid != 0) tasks_->Remove(tid);
        return 0;
      }
      if (id == IDC_BTN_SAVE) {
        tasks_->Persist();
        return 0;
      }
      break;
    }
    case WM_TIMER:
      if (wParam == TIMER_ID_MAIN) {
        tasks_->OnTimerTick();
      }
      return 0;
    case WM_APP_REFRESH_LIST:
      RefreshList();
      return 0;
    case WM_APP_LLM_RESULT: {
      auto* payload = reinterpret_cast<LlmMessagePayload*>(lParam);
      if (payload) {
        tasks_->OnLlmSuccess(payload->taskId);
        redis_->Append(payload->taskId, payload->promptUtf8, payload->responseUtf8);
        MessageBoxW(hwnd_, payload->userVisibleText.c_str(), L"LLM", MB_OK | MB_ICONINFORMATION);
        delete payload;
      }
      RefreshList();
      return 0;
    }
    case WM_APP_LLM_ERROR: {
      auto* payload = reinterpret_cast<LlmMessagePayload*>(lParam);
      if (payload) {
        tasks_->OnLlmError(payload->taskId);
        const std::int64_t now = NowUtcMs();
        const std::int64_t intervalMs =
            static_cast<std::int64_t>(config_.llmErrorLogIntervalSec) * 1000;
        if (now - lastLlmErrorLogMs_ >= intervalMs) {
          lastLlmErrorLogMs_ = now;
          logx::Logger::Instance().Error("LLM: " + util::WideToUtf8(payload->userVisibleText));
        }
        delete payload;
      }
      return 0;
    }
    case WM_DESTROY:
      timer_.Stop(hwnd_, TIMER_ID_MAIN);
      PostQuitMessage(0);
      return 0;
    default:
      break;
  }
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}
