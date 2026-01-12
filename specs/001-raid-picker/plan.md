# Implementation Plan: Raid Picker Plugin

**Branch**: `001-raid-picker` | **Date**: 2026-01-11 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-raid-picker/spec.md`

## Summary

Build an OBS Studio native plugin that enables streamers to discover and raid other streamers using configurable filters. The plugin provides preset filters (Beginner/Small/Medium), custom filter creation, followed-streamer raiding, greylist management, and preview-before-raid functionality. Implemented as a native C/C++ OBS plugin using the OBS Plugin SDK with Qt UI for optimal performance and native integration.

## Technical Context

**Language/Version**: C++17 (OBS Studio requirement)
**Primary Dependencies**: OBS Plugin SDK, Qt 6 (OBS UI framework), libcurl (HTTP requests), nlohmann/json (JSON parsing)
**Storage**: OBS settings API (`obs_data_t`) for configuration persistence
**Testing**: Google Test for unit tests, manual OBS integration testing
**Target Platform**: Windows 10+, macOS 10.15+, Linux (Ubuntu 20.04+)
**Project Type**: Single project (native OBS plugin)
**Performance Goals**: <100ms UI response, <3s API calls, zero main-thread blocking
**Constraints**: Must not block OBS main thread, <50MB memory footprint, async network I/O
**Scale/Scope**: Single-user desktop plugin, ~5-10 API calls per raid session

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Requirement | Status |
|-----------|-------------|--------|
| I. Simplicity First | All features solve immediate user needs | ✅ PASS - All 5 user stories address concrete streaming scenarios |
| I. Simplicity First | Simplest solution preferred | ✅ PASS - Native plugin is simplest for OBS integration |
| II. OBS Integration | UI follows OBS conventions | ✅ PASS - Using Qt 6 for native look |
| II. OBS Integration | No main-thread blocking | ✅ PASS - Async HTTP via worker threads |
| II. OBS Integration | State persists via OBS data settings | ✅ PASS - Using obs_data_t API |
| II. OBS Integration | Localization support | ✅ PASS - Qt i18n framework |
| III. Privacy & Security | OAuth tokens stored securely | ✅ PASS - Using OS keychain/credential store |
| III. Privacy & Security | Greylist local only | ✅ PASS - Stored in OBS config |
| III. Privacy & Security | HTTPS only | ✅ PASS - Twitch API requires HTTPS |
| IV. User Experience | Preview before raid | ✅ PASS - Core feature (US1) |
| IV. User Experience | Clear preset criteria | ✅ PASS - Documented in spec assumptions |
| IV. User Experience | Actionable error feedback | ✅ PASS - Edge cases defined |
| V. Maintainability | Readable code | ✅ PASS - C++17 modern patterns |
| V. Maintainability | Dependencies justified | ✅ PASS - All are standard OBS ecosystem |

**Gate Result**: ✅ ALL GATES PASS - Proceed to Phase 0

## Project Structure

### Documentation (this feature)

```text
specs/001-raid-picker/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (Twitch API contracts)
└── tasks.md             # Phase 2 output (/speckit.tasks command)
```

### Source Code (repository root)

```text
src/
├── plugin.cpp           # OBS plugin entry point
├── plugin.h
├── ui/
│   ├── raid-picker-dock.cpp    # Main dockable panel
│   ├── raid-picker-dock.h
│   ├── preset-selector.cpp     # Preset dropdown component
│   ├── filter-editor.cpp       # Custom filter UI
│   ├── streamer-preview.cpp    # Preview card component
│   └── greylist-manager.cpp    # Greylist management dialog
├── services/
│   ├── twitch-api.cpp          # Twitch Helix API client
│   ├── twitch-api.h
│   ├── oauth-manager.cpp       # OAuth 2.0 flow handler
│   ├── oauth-manager.h
│   └── async-worker.cpp        # Background thread pool
├── models/
│   ├── streamer.cpp            # Streamer data model
│   ├── filter-preset.cpp       # Filter preset model
│   ├── greylist-entry.cpp      # Greylist entry model
│   └── session-state.cpp       # Runtime state
├── storage/
│   ├── settings-manager.cpp    # OBS data persistence
│   └── credential-store.cpp    # Secure token storage
└── i18n/
    ├── en-US.json              # English translations
    └── fr-FR.json              # French translations

tests/
├── unit/
│   ├── test-filter-logic.cpp
│   ├── test-streamer-model.cpp
│   └── test-preset-validation.cpp
└── integration/
    └── test-twitch-api-mock.cpp

cmake/
├── CMakeLists.txt
└── FindOBS.cmake
```

**Structure Decision**: Single project structure with clear separation between UI (Qt), services (API/async), models (data), and storage (persistence). This aligns with OBS plugin conventions and keeps the codebase navigable.

## Complexity Tracking

> No violations - all choices align with constitution principles.

| Choice | Justification |
|--------|---------------|
| C++17 | Required by OBS Plugin SDK; modern features reduce boilerplate |
| Qt 6 | Native OBS UI framework; no additional dependency |
| libcurl | Industry-standard HTTP; bundled with most OS |
| nlohmann/json | Header-only, widely used, simple API |
