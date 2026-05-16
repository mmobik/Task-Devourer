#include "llm/LlmClient.h"

#include <chrono>
#include <thread>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "app/AppMessages.h"
#include "util/StringUtil.h"

namespace llm {

LlmClient::LlmClient(const AppConfig& config) : config_(config) {}

void LlmClient::AskOverdueHelp(HWND hwnd, const std::uint64_t taskId,
                               const std::string& taskTextUtf8) {
  bool expected = false;
  if (!inFlight_.compare_exchange_strong(expected, true)) {
    return;
  }

  const AppConfig cfg = config_;
  const std::string promptUtf8 =
      "\xD0\x97\xD0\xB0\xD0\xB4\xD0\xB0\xD1\x87\xD0\xB0 \xC2\xAB" + taskTextUtf8 +
      "\xC2\xBB \xD0\xBD\xD0\xB5 \xD0\xB2\xD1\x8B\xD0\xBF\xD0\xBE\xD0\xBB\xD0\xBD\xD0\xB5\xD0\xBD\xD0\xB0. "
      "\xD0\x9F\xD0\xBE\xD1\x87\xD0\xB5\xD0\xBC\xD1\x83?";

  LlmClient* self = this;
  std::thread([hwnd, cfg, taskId, promptUtf8, self]() {
    std::string error;
    std::string assistant;
    try {
      httplib::Client cli(cfg.ollamaHost, cfg.ollamaPort);
      cli.set_connection_timeout(cfg.llmHttpTimeoutSec, 0);
      cli.set_read_timeout(cfg.llmHttpTimeoutSec, 0);
      cli.set_write_timeout(cfg.llmHttpTimeoutSec, 0);

      nlohmann::json body;
      body["model"] = cfg.ollamaModel;
      body["stream"] = false;
      body["messages"] = nlohmann::json::array({
          {{"role", "user"}, {"content", promptUtf8}},
      });

      const auto res = cli.Post("/api/chat", body.dump(), "application/json");
      if (!res) {
        error = "HTTP request failed";
      } else if (res->status < 200 || res->status >= 300) {
        error = "HTTP status " + std::to_string(res->status) + ": " + res->body;
      } else {
        const auto j = nlohmann::json::parse(res->body);
        if (j.contains("message") && j["message"].contains("content")) {
          assistant = j["message"]["content"].get<std::string>();
        } else {
          error = "unexpected Ollama response";
        }
      }
    } catch (const std::exception& ex) {
      error = ex.what();
    }

    self->inFlight_.store(false);

    if (!error.empty()) {
      auto* payload = new LlmMessagePayload{taskId, util::Utf8ToWide(error), promptUtf8, ""};
      PostMessageW(hwnd, WM_APP_LLM_ERROR, 0, reinterpret_cast<LPARAM>(payload));
    } else {
      auto* payload =
          new LlmMessagePayload{taskId, util::Utf8ToWide(assistant), promptUtf8, assistant};
      PostMessageW(hwnd, WM_APP_LLM_RESULT, 0, reinterpret_cast<LPARAM>(payload));
    }
  }).detach();
}

}  // namespace llm
