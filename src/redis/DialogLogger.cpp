#include "redis/DialogLogger.h"

#include <chrono>
#include <hiredis/hiredis.h>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

#include "log/Logger.h"

namespace redislog {

namespace {

std::string NowIsoUtc() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  gmtime_s(&tm, &t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
  return oss.str();
}

}  // namespace

DialogLogger::DialogLogger(const AppConfig& config) : config_(config) {}

void DialogLogger::EnsureConnected() {
  if (context_ != nullptr) return;
  auto* ctx = redisConnect(config_.redisHost.c_str(), config_.redisPort);
  if (ctx == nullptr || ctx->err) {
    if (ctx != nullptr) {
      logx::Logger::Instance().Warn(std::string("redis connect: ") + ctx->errstr);
      redisFree(ctx);
    } else {
      logx::Logger::Instance().Warn("redis connect: allocation failed");
    }
    return;
  }
  context_ = ctx;
}

void DialogLogger::Append(const std::uint64_t taskId, const std::string& promptUtf8,
                          const std::string& responseUtf8) {
  EnsureConnected();
  if (context_ == nullptr) return;

  nlohmann::json entry;
  entry["at"] = NowIsoUtc();
  entry["prompt"] = promptUtf8;
  entry["response"] = responseUtf8;

  const std::string key = config_.redisKeyPrefix + std::to_string(taskId);
  auto* ctx = static_cast<redisContext*>(context_);
  redisReply* reply = static_cast<redisReply*>(
      redisCommand(ctx, "LPUSH %s %s", key.c_str(), entry.dump().c_str()));
  if (reply == nullptr) {
    logx::Logger::Instance().Warn("redis LPUSH failed");
  } else {
    freeReplyObject(reply);
  }
}

}  // namespace redislog
