# Architecture

```
MainWindow (UI) ──► TaskService ◄── TimerService (SetTimer)
                         │
                         ├── JsonStorage → tasks.json (texts only)
                         ├── LlmClient → Ollama HTTP (std::async)
                         └── DialogLogger → Redis LIST (optional)
```

- **tasks.json**: `{"tasks":["..."]}` — strings only.
- **config.json**: timer, Ollama, Redis settings (copy from `config.json.example`).
- **app.log**: runtime log next to the executable.

See the project README for build and run instructions.
