#pragma once

#include <mutex>
#include <string>

namespace logx {

class Logger {
 public:
  static Logger& Instance();

  void SetLogPath(const std::wstring& path);
  void Info(const std::string& message);
  void Warn(const std::string& message);
  void Error(const std::string& message);

 private:
  Logger() = default;
  void WriteLine(const char* level, const std::string& message);

  std::mutex mutex_;
  std::wstring path_;
};

}  // namespace logx
