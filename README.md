# Task-Devourer

Win32 task manager (C++17) with Ollama LLM checks and optional Redis dialog history.

## Prerequisites

- Windows 10/11
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.20+
- [Ollama](https://ollama.com/) on `127.0.0.1:11434`
- Optional: Redis on `127.0.0.1:6379`

## Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Output: `build\Release\TaskDevourer.exe` (static CRT `/MT`).

First configure downloads dependencies via CMake FetchContent (needs network).

## Run

1. Copy `config.json.example` next to the exe as `config.json` (or let the app copy it on first start).
2. Start Ollama; pull the model from config (default `llama3.2`).
3. Optionally start Redis.
4. Run `TaskDevourer.exe`.

Files beside the exe:

| File | Purpose |
|------|---------|
| `config.json` | Timer, Ollama, Redis |
| `tasks.json` | Task texts only: `{"tasks":["..."]}` |
| `app.log` | Application log |

**Note:** Deadlines, status, and check intervals live in memory only; restarting resets metadata except task text lines in `tasks.json`.

## Behavior

- Set deadline and **check minutes** before Add.
- Timer marks tasks overdue; after deadline + check minutes, Ollama is asked once per overdue period (Russian prompt).
- LLM errors: logged to `app.log` every 30s (configurable), no popup.
- LLM success: `MessageBox` + Redis `LPUSH` when Redis is up.

See [CONTRIBUTING.md](CONTRIBUTING.md) and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
