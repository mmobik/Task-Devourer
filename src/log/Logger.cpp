#include "log/Logger.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace logx {

Logger& Logger::Instance() {
  static Logger inst;
  return inst;
}

void Logger::SetLogPath(const std::wstring& path) {
  std::lock_guard<std::mutex> lock(mutex_);
  path_ = path;
}

void Logger::Info(const std::string& message) { WriteLine("INFO", message); }

void Logger::Warn(const std::string& message) { WriteLine("WARN", message); }

void Logger::Error(const std::string& message) { WriteLine("ERROR", message); }

void Logger::WriteLine(const char* level, const std::string& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (path_.empty()) {
    return;
  }
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  localtime_s(&tm, &t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [" << level << "] " << message << '\n';
  std::ofstream out(path_, std::ios::app);
  if (out) {
    out << oss.str();
  }
}

}  // namespace logx
