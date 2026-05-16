#include "storage/JsonStorage.h"

#include <windows.h>

#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

#include "core/Task.h"
#include "log/Logger.h"

namespace storage {

JsonStorage::JsonStorage(std::wstring path) : path_(std::move(path)) {}

bool JsonStorage::Load(std::vector<std::string>& titlesUtf8) {
  titlesUtf8.clear();
  std::ifstream in(path_);
  if (!in) {
    return true;
  }
  try {
    nlohmann::json j;
    in >> j;
    if (!j.contains("tasks") || !j["tasks"].is_array()) {
      throw std::runtime_error("missing tasks array");
    }
    for (const auto& item : j["tasks"]) {
      if (item.is_string()) {
        titlesUtf8.push_back(item.get<std::string>());
      }
    }
    return true;
  } catch (const std::exception& ex) {
    logx::Logger::Instance().Warn(std::string("tasks.json corrupt: ") + ex.what());
    const std::wstring backup = path_ + L".corrupt";
    CopyFileW(path_.c_str(), backup.c_str(), FALSE);
    std::ofstream out(path_);
    out << R"({"tasks":[]})";
    titlesUtf8.clear();
    return false;
  }
}

bool JsonStorage::Save(const std::vector<Task>& tasks) {
  nlohmann::json j;
  j["tasks"] = nlohmann::json::array();
  for (const auto& t : tasks) {
    j["tasks"].push_back(t.text);
  }
  std::ofstream out(path_);
  if (!out) {
    logx::Logger::Instance().Error("failed to write tasks.json");
    return false;
  }
  out << j.dump(2);
  return true;
}

}  // namespace storage
