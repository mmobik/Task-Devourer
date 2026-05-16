#pragma once

#include <string>
#include <vector>

#include "core/Task.h"

namespace storage {

class JsonStorage {
 public:
  explicit JsonStorage(std::wstring path);
  bool Load(std::vector<std::string>& titlesUtf8);
  bool Save(const std::vector<Task>& tasks);

 private:
  std::wstring path_;
};

}  // namespace storage
