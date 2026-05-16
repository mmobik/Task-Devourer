# Architecture

```
MainWindow (UI) --> TaskService <-- TimerService (SetTimer)
                        |
                        +-- JsonStorage --> tasks.json (texts only)
                        +-- LlmClient --> Ollama /api/chat (std::thread)
                        +-- DialogLogger --> Redis LIST (optional)
```

- **tasks.json**: `{"tasks":["title", ...]}` only.
- In-memory: numeric `id`, deadline, status, check minutes, LLM flags.
- **config.json** / **app.log** next to the executable.
