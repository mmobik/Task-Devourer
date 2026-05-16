#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <commctrl.h>

#include "gui/MainWindow.h"

#pragma comment(lib, "ws2_32.lib")

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCmd) {
  WSADATA wsa{};
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    return 1;
  }

  INITCOMMONCONTROLSEX icc{};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_DATE_CLASSES | ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&icc);

  MainWindow window;
  const bool ok = window.Create(instance, showCmd);

  int code = 0;
  if (ok) {
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    code = static_cast<int>(msg.wParam);
  } else {
    code = 1;
  }

  WSACleanup();
  return code;
}
