#include "util/Paths.h"

#include <windows.h>

namespace util {

std::wstring GetExeDirectory() {
  wchar_t buf[MAX_PATH]{};
  const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) {
    return L".\\";
  }
  std::wstring path(buf, n);
  const auto pos = path.find_last_of(L"\\/");
  if (pos == std::wstring::npos) {
    return L".\\";
  }
  return path.substr(0, pos + 1);
}

}  // namespace util
