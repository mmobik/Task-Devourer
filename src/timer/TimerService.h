#pragma once

#include <windows.h>

namespace timerx {

class TimerService {
 public:
  void Start(HWND hwnd, UINT_PTR id, UINT intervalMs);
  void Stop(HWND hwnd, UINT_PTR id);
};

}  // namespace timerx
