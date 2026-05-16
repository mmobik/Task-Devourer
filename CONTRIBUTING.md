# Contributing to Task-Devourer

## Branches

- `main` — always buildable Release configuration.
- `feature/<short-name>` — new functionality (e.g. `feature/win32-gui`).
- `fix/<short-name>` — bug fixes.

Create a feature branch from `main`, merge back after local `cmake --build` succeeds.

## Commits

Use [Conventional Commits](https://www.conventionalcommits.org/) with English subjects:

- `feat: add TaskService timer tick logic`
- `fix: guard MessageBox to UI thread`
- `chore: add CMake project scaffold`
- `docs: README build instructions`

One logical change per commit. Do not commit `build/`, `out/`, `*.user`, `app.log`, or local `config.json`.

## Code style

- C++17, MSVC 2022.
- UTF-8 in `std::string`; convert to wide strings only at the Win32 UI boundary.
- UI thread owns `TaskService` mutations; HTTP runs in `std::async` workers.
