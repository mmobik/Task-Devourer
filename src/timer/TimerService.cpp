#include "timer/TimerService.h"

namespace timerx {

void TimerService::Start(HWND hwnd, UINT_PTR id, UINT intervalMs) {
  SetTimer(hwnd, id, intervalMs, nullptr);
}

void TimerService::Stop(HWND hwnd, UINT_PTR id) { KillTimer(hwnd, id); }

}  // namespace timerx
