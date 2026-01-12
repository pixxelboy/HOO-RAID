# Tasks: Raid Picker Plugin

**Input**: Design documents from `/specs/001-raid-picker/`
**Prerequisites**: plan.md, spec.md, data-model.md, contracts/, research.md, quickstart.md

**Tests**: Not explicitly requested in specification. Manual OBS integration testing per quickstart.md.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `tests/` at repository root (OBS plugin structure)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [x] T001 Create project structure per implementation plan in src/, tests/, cmake/
- [x] T002 Initialize CMake build system with OBS Plugin SDK in cmake/CMakeLists.txt
- [x] T003 [P] Add FindOBS.cmake module for OBS SDK discovery in cmake/FindOBS.cmake
- [x] T004 [P] Configure nlohmann/json dependency in cmake/CMakeLists.txt
- [x] T005 [P] Configure QtKeychain dependency for credential storage in cmake/CMakeLists.txt
- [x] T006 [P] Configure libcurl dependency for HTTP requests in cmake/CMakeLists.txt
- [x] T007 [P] Create English translation file in src/i18n/en-US.json
- [x] T008 [P] Create French translation file in src/i18n/fr-FR.json

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

### Plugin Entry Point

- [x] T009 Create OBS plugin entry point with obs_module_load/unload in src/plugin.cpp
- [x] T010 [P] Create plugin header with module declarations in src/plugin.h

### Core Models (shared by all stories)

- [x] T011 [P] Implement Streamer model with JSON parsing in src/models/streamer.cpp
- [x] T012 [P] Implement Streamer header with struct definition in src/models/streamer.h
- [x] T013 [P] Implement FilterPreset model with built-in presets in src/models/filter-preset.cpp
- [x] T014 [P] Implement FilterPreset header with matches() method in src/models/filter-preset.h
- [x] T015 [P] Implement OAuthCredentials model in src/models/oauth-credentials.cpp
- [x] T016 [P] Implement OAuthCredentials header in src/models/oauth-credentials.h
- [x] T017 [P] Implement SessionState model in src/models/session-state.cpp
- [x] T018 [P] Implement SessionState header in src/models/session-state.h

### Credential Storage

- [x] T019 Implement ICredentialStore interface header in src/storage/credential-store.h
- [x] T020 Implement CredentialStore with QtKeychain in src/storage/credential-store.cpp

### OAuth Manager

- [x] T021 Implement IOAuthManager interface header in src/services/oauth-manager.h
- [x] T022 Implement OAuthManager with auth flow, token refresh in src/services/oauth-manager.cpp

### Async Worker Infrastructure

- [x] T023 Implement AsyncWorker thread pool for non-blocking operations in src/services/async-worker.cpp
- [x] T024 [P] Implement AsyncWorker header with signal/slot interface in src/services/async-worker.h

### Base UI Dock

- [x] T025 Implement RaidPickerDock base class as QDockWidget in src/ui/raid-picker-dock.cpp
- [x] T026 [P] Implement RaidPickerDock header in src/ui/raid-picker-dock.h
- [x] T027 Register dock widget with OBS main window in src/plugin.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin

---

## Phase 3: User Story 1 - Quick Raid with Preset (Priority: P1) 🎯 MVP

**Goal**: Enable streamers to quickly find and raid a smaller streamer using preset filters

**Independent Test**: Select a preset, click "Pick Random", preview streamer details, execute raid

### Twitch API Client

- [x] T028 [US1] Implement ITwitchApi interface header in src/services/twitch-api.h
- [x] T029 [US1] Implement TwitchApi::fetchStreams with language/game filters in src/services/twitch-api.cpp
- [x] T030 [US1] Implement TwitchApi::fetchFollowerCount for streamer lookup in src/services/twitch-api.cpp
- [x] T031 [US1] Implement TwitchApi::startRaid POST /raids endpoint in src/services/twitch-api.cpp
- [x] T032 [US1] Implement TwitchApi::cancelRaid DELETE /raids endpoint in src/services/twitch-api.cpp
- [x] T033 [US1] Add rate limit handling with Ratelimit-Reset header parsing in src/services/twitch-api.cpp

### Streamer Selector

- [x] T034 [US1] Implement IStreamerSelector interface header in src/services/streamer-selector.h
- [x] T035 [US1] Implement StreamerSelector with Fisher-Yates shuffle in src/services/streamer-selector.cpp
- [x] T036 [US1] Add selectNext() for re-roll without re-query in src/services/streamer-selector.cpp

### UI Components

- [x] T037 [US1] Implement PresetSelector dropdown with Beginner/Small/Medium in src/ui/preset-selector.cpp
- [x] T038 [P] [US1] Implement PresetSelector header in src/ui/preset-selector.h
- [x] T039 [US1] Implement StreamerPreview card with title, category, viewers, followers in src/ui/streamer-preview.cpp
- [x] T040 [P] [US1] Implement StreamerPreview header in src/ui/streamer-preview.h
- [x] T041 [US1] Add "Pick Random" button to RaidPickerDock in src/ui/raid-picker-dock.cpp
- [x] T042 [US1] Add "Pick Another" re-roll button to StreamerPreview in src/ui/streamer-preview.cpp
- [x] T043 [US1] Add "Launch Raid" button to StreamerPreview in src/ui/streamer-preview.cpp
- [x] T044 [US1] Add loading indicator during API calls in src/ui/raid-picker-dock.cpp
- [x] T045 [US1] Add error display with retry option in src/ui/raid-picker-dock.cpp

### Integration

- [x] T046 [US1] Wire PresetSelector to TwitchApi::fetchStreams in src/ui/raid-picker-dock.cpp
- [x] T047 [US1] Wire StreamerPreview to TwitchApi::startRaid in src/ui/raid-picker-dock.cpp
- [x] T048 [US1] Display raid confirmation/error feedback in src/ui/raid-picker-dock.cpp

**Checkpoint**: User Story 1 complete - Can discover and raid streamers with presets

---

## Phase 4: User Story 2 - Custom Filter Raid (Priority: P2)

**Goal**: Allow streamers to configure custom filters for precise streamer discovery

**Independent Test**: Set custom filters (language, category, viewer/follower ranges), save as preset, verify filtering works

### Game Lookup

- [x] T049 [US2] Implement TwitchApi::searchGames for category autocomplete in src/services/twitch-api.cpp
- [x] T050 [P] [US2] Add Game model struct for category data in src/models/game.h

### Filter Editor UI

- [x] T051 [US2] Implement FilterEditor dialog with language dropdown in src/ui/filter-editor.cpp
- [x] T052 [P] [US2] Implement FilterEditor header in src/ui/filter-editor.h
- [x] T053 [US2] Add category/game autocomplete field to FilterEditor in src/ui/filter-editor.cpp
- [x] T054 [US2] Add viewer range inputs (min/max) to FilterEditor in src/ui/filter-editor.cpp
- [x] T055 [US2] Add follower range inputs (min/max) to FilterEditor in src/ui/filter-editor.cpp
- [x] T056 [US2] Add preset validation (min <= max, at least one filter) in src/ui/filter-editor.cpp
- [x] T057 [US2] Add "Save as Preset" button with name input in src/ui/filter-editor.cpp

### Custom Preset Management

- [x] T058 [US2] Add custom presets to PresetSelector dropdown in src/ui/preset-selector.cpp
- [x] T059 [US2] Add "Edit" button for custom presets in PresetSelector in src/ui/preset-selector.cpp
- [x] T060 [US2] Add "Delete" button for custom presets in PresetSelector in src/ui/preset-selector.cpp

### Empty State

- [x] T061 [US2] Display "No matches found" with filter suggestions in src/ui/raid-picker-dock.cpp

**Checkpoint**: User Story 2 complete - Can create and use custom filter presets

---

## Phase 5: User Story 3 - Raid Followed Streamers (Priority: P3)

**Goal**: Enable raiding from the user's followed streamers list

**Independent Test**: Select "Raid from Followed", see live followed streamers, apply filters, pick random

### Followed Streams API

- [x] T062 [US3] Implement TwitchApi::fetchFollowedStreams endpoint in src/services/twitch-api.cpp

### UI Components

- [x] T063 [US3] Add "Raid from Followed" mode toggle to RaidPickerDock in src/ui/raid-picker-dock.cpp
- [x] T064 [US3] Display followed streamers in selection pool in src/ui/raid-picker-dock.cpp
- [x] T065 [US3] Apply existing filters to followed streamers list in src/ui/raid-picker-dock.cpp

### Empty State

- [x] T066 [US3] Display "No followed streamers are live" message in src/ui/raid-picker-dock.cpp

**Checkpoint**: User Story 3 complete - Can raid from followed streamers

---

## Phase 6: User Story 4 - Greylist Management (Priority: P4)

**Goal**: Allow users to exclude specific streamers from raid suggestions

**Independent Test**: Add streamer to greylist from preview, verify they don't appear in picks, manage greylist

### Greylist Model

- [x] T067 [US4] Implement GreylistEntry model in src/models/greylist-entry.cpp
- [x] T068 [P] [US4] Implement GreylistEntry header in src/models/greylist-entry.h

### Greylist Logic

- [x] T069 [US4] Add greylist exclusion to StreamerSelector::setPool in src/services/streamer-selector.cpp
- [x] T070 [US4] Add excludeFromPool() for immediate removal in src/services/streamer-selector.cpp

### UI Components

- [x] T071 [US4] Add "Add to Greylist" button to StreamerPreview in src/ui/streamer-preview.cpp
- [x] T072 [US4] Implement GreylistManager dialog with list view in src/ui/greylist-manager.cpp
- [x] T073 [P] [US4] Implement GreylistManager header in src/ui/greylist-manager.h
- [x] T074 [US4] Add remove button per entry in GreylistManager in src/ui/greylist-manager.cpp
- [x] T075 [US4] Add "Manage Greylist" button to RaidPickerDock in src/ui/raid-picker-dock.cpp

### Integration

- [x] T076 [US4] Auto-advance to next streamer after greylisting in src/ui/raid-picker-dock.cpp

**Checkpoint**: User Story 4 complete - Can manage greylist

---

## Phase 7: User Story 5 - Persistent Settings (Priority: P5)

**Goal**: Persist authentication, greylist, and custom presets across OBS restarts

**Independent Test**: Configure settings, restart OBS, verify all settings restored

### Settings Manager

- [x] T077 [US5] Implement ISettingsManager interface header in src/storage/settings-manager.h
- [x] T078 [US5] Implement SettingsManager with obs_data_t API in src/storage/settings-manager.cpp
- [x] T079 [US5] Add saveCustomPreset/deleteCustomPreset methods in src/storage/settings-manager.cpp
- [x] T080 [US5] Add greylist persistence (add/remove/get) in src/storage/settings-manager.cpp
- [x] T081 [US5] Add user info caching (userId, userLogin) in src/storage/settings-manager.cpp
- [x] T082 [US5] Add UI state persistence (lastPresetId) in src/storage/settings-manager.cpp

### Integration

- [x] T083 [US5] Load settings on plugin startup in src/plugin.cpp
- [x] T084 [US5] Save settings on plugin unload in src/plugin.cpp
- [x] T085 [US5] Wire SettingsManager to PresetSelector for custom preset persistence in src/ui/preset-selector.cpp
- [x] T086 [US5] Wire SettingsManager to GreylistManager for greylist persistence in src/ui/greylist-manager.cpp
- [x] T087 [US5] Restore OAuth session from stored credentials on startup in src/services/oauth-manager.cpp

**Checkpoint**: User Story 5 complete - All settings persist across restarts

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [x] T088 [P] Add logging throughout services using blog() in src/services/*.cpp
- [x] T089 [P] Add error message localization in src/i18n/*.json
- [x] T090 Implement token refresh on 401 response in src/services/twitch-api.cpp
- [x] T091 Add "Logout" button to RaidPickerDock in src/ui/raid-picker-dock.cpp
- [x] T092 Add rate limit user feedback (retry countdown) in src/ui/raid-picker-dock.cpp
- [x] T093 [P] Update quickstart.md with build verification steps in specs/001-raid-picker/quickstart.md
- [ ] T094 Verify plugin loads correctly in OBS (manual test per quickstart.md)
- [ ] T095 Verify all user stories work end-to-end (manual test per spec.md acceptance scenarios)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phases 3-7)**: All depend on Foundational phase completion
  - User stories can proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3 → P4 → P5)
- **Polish (Phase 8)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational - Uses US1 TwitchApi, extends PresetSelector
- **User Story 3 (P3)**: Can start after Foundational - Uses US1 TwitchApi and StreamerSelector
- **User Story 4 (P4)**: Can start after Foundational - Uses US1 StreamerPreview and StreamerSelector
- **User Story 5 (P5)**: Can start after Foundational - Integrates with US2 presets, US4 greylist

### Within Each User Story

- Models before services
- Services before UI components
- Core implementation before integration
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks T003-T008 can run in parallel
- All Foundational model tasks T011-T018 can run in parallel
- Within US1: T038, T040 can run parallel to their implementation files
- Within US2: T050, T052 can run parallel
- Within US4: T068, T073 can run parallel
- Once Foundational completes, multiple user stories can start in parallel

---

## Parallel Example: User Story 1

```bash
# Launch all parallel tasks for User Story 1 together:
Task T038: "Implement PresetSelector header in src/ui/preset-selector.h"
Task T040: "Implement StreamerPreview header in src/ui/streamer-preview.h"

# After headers complete, implementation tasks can proceed
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test in OBS - can discover and raid streamers with presets
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → **MVP Ready!**
3. Add User Story 2 → Test independently → Custom filters work
4. Add User Story 3 → Test independently → Followed streamers work
5. Add User Story 4 → Test independently → Greylist works
6. Add User Story 5 → Test independently → Persistence works
7. Polish phase → Production ready

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (MVP)
   - Developer B: User Story 2 + User Story 3
   - Developer C: User Story 4 + User Story 5
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
- Manual testing in OBS required - see quickstart.md for setup instructions
