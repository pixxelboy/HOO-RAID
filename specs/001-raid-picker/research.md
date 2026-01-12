# Research: Raid Picker Plugin

**Feature**: 001-raid-picker
**Date**: 2026-01-11
**Status**: Complete

## Research Areas

1. Twitch Helix API - Stream Discovery & Raids
2. OBS Plugin SDK - Dockable Panel Architecture
3. Secure Credential Storage - Cross-Platform OAuth Token Management

---

## 1. Twitch Helix API

### 1.1 Get Streams Endpoint

**Decision**: Use `GET https://api.twitch.tv/helix/streams` with client-side filtering for viewer/follower ranges.

**Rationale**: The Twitch API does not support filtering by viewer count or follower count directly. We must fetch streams by language/game and apply range filters client-side.

**Endpoint**: `GET https://api.twitch.tv/helix/streams`

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| `game_id` | string | Filter by game/category ID (use Get Games to resolve names) |
| `language` | string | Filter by ISO 639-1 language code (e.g., "en", "fr") |
| `first` | integer | Page size (default 20, max 100) |
| `after` | string | Pagination cursor |

**Response Fields** (per stream):
- `id` - Stream ID
- `user_id` - Broadcaster user ID
- `user_login` - Broadcaster login name
- `user_name` - Broadcaster display name
- `game_id` - Current game ID
- `game_name` - Current game name
- `title` - Stream title
- `viewer_count` - Current viewers
- `started_at` - Stream start timestamp
- `language` - Broadcast language
- `thumbnail_url` - Stream thumbnail

**Note**: Follower count is NOT included in stream data. Requires separate API call.

**Alternatives Considered**:
- Search Channels endpoint - Mixes live/offline data, less suitable for discovery
- EventSub subscriptions - Overkill for on-demand queries

### 1.2 Get Channel Followers Endpoint

**Decision**: Use `GET https://api.twitch.tv/helix/channels/followers` to get follower counts.

**Rationale**: This is the current supported endpoint (replaces deprecated `/helix/users/follows`).

**Endpoint**: `GET https://api.twitch.tv/helix/channels/followers`

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| `broadcaster_id` | string | Required - Channel to get followers for |
| `first` | integer | Page size (max 100) |

**Response**:
- `total` - Total follower count (available without special scope)
- `data[]` - Follower details (requires `moderator:read:followers` scope)

**Important**: Without the `moderator:read:followers` scope, we can still get the `total` count, which is sufficient for filtering purposes.

### 1.3 Get Followed Streams Endpoint

**Decision**: Use `GET https://api.twitch.tv/helix/streams/followed` for "Raid from Followed" feature.

**Rationale**: Single endpoint returns only live followed channels.

**Endpoint**: `GET https://api.twitch.tv/helix/streams/followed`

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| `user_id` | string | Required - Must match OAuth token user ID |
| `first` | integer | Page size (default 20, max 100) |
| `after` | string | Pagination cursor |

**Required Scope**: `user:read:follows`

### 1.4 Start Raid Endpoint

**Decision**: Use `POST https://api.twitch.tv/helix/raids` to initiate raids.

**Rationale**: Official API endpoint for programmatic raids.

**Endpoint**: `POST https://api.twitch.tv/helix/raids`

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| `from_broadcaster_id` | string | Query param - Raiding channel (must match token) |
| `to_broadcaster_id` | string | Query param - Target channel |

**Required Scope**: `channel:manage:raids`

**Response**: Returns raid creation time. Does NOT confirm raid executed.

**Important Notes**:
- Rate limit: 10 requests per 10-minute window
- Raid is queued, not immediately executed
- User sees 90-second countdown in Twitch UI
- Can be cancelled before countdown completes

### 1.5 Cancel Raid Endpoint

**Decision**: Include cancel functionality for user control.

**Endpoint**: `DELETE https://api.twitch.tv/helix/raids`

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| `broadcaster_id` | string | Query param - Must match token user |

**Required Scope**: `channel:manage:raids`

### 1.6 Get Games Endpoint (for category lookup)

**Decision**: Use `GET https://api.twitch.tv/helix/games` to resolve category names to IDs.

**Endpoint**: `GET https://api.twitch.tv/helix/games`

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | string | Category/game name (partial match supported) |
| `id` | string | Game ID (for reverse lookup) |

### 1.7 OAuth Scopes Summary

**Required Scopes**:
| Scope | Purpose |
|-------|---------|
| `user:read:follows` | Get followed streams |
| `channel:manage:raids` | Start/cancel raids |

**Optional Scope**:
| Scope | Purpose |
|-------|---------|
| `moderator:read:followers` | Get detailed follower data (not needed, `total` is public) |

### 1.8 Rate Limits

- Standard: 800 requests per minute (app access token)
- User tokens: 800 requests per minute per user
- Raids endpoint: 10 requests per 10 minutes
- Recommendation: Cache stream results, batch follower lookups

---

## 2. OBS Plugin SDK

### 2.1 Plugin Architecture

**Decision**: Create native C++ plugin with dockable Qt panel.

**Rationale**: Native integration provides best UX and follows OBS conventions.

**Required Entry Points**:
```cpp
// plugin.cpp
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("hoo-raid", "en-US")

bool obs_module_load(void) {
    // Register dockable panel
    return true;
}

void obs_module_unload(void) {
    // Cleanup
}
```

### 2.2 Dockable Panel Implementation

**Decision**: Use QDockWidget attached to OBS main window.

**Implementation Pattern**:
```cpp
// In obs_module_load()
QMainWindow *main_window = static_cast<QMainWindow*>(obs_frontend_get_main_window());

RaidPickerDock *dock = new RaidPickerDock(main_window);
dock->setFeatures(QDockWidget::DockWidgetClosable |
                  QDockWidget::DockWidgetMovable |
                  QDockWidget::DockWidgetFloatable);
dock->setWindowTitle(obs_module_text("RaidPicker"));

main_window->addDockWidget(Qt::RightDockWidgetArea, dock);
```

**Alternatives Considered**:
- Browser source panel - More complex deployment, CORS issues
- Modal dialog - Poor UX for frequent use
- Source properties - Doesn't fit use case

### 2.3 Settings Persistence

**Decision**: Use `obs_data_t` API for configuration storage.

**Implementation Pattern**:
```cpp
// Save settings
obs_data_t *settings = obs_data_create();
obs_data_set_string(settings, "custom_presets_json", presets_json);
obs_data_set_string(settings, "greylist_json", greylist_json);

char *path = obs_module_config_path("settings.json");
obs_data_save_json(settings, path);
bfree(path);
obs_data_release(settings);

// Load settings
char *path = obs_module_config_path("settings.json");
obs_data_t *settings = obs_data_create_from_json_file(path);
// ... read values
bfree(path);
obs_data_release(settings);
```

### 2.4 Async Operations

**Decision**: Use Qt's threading model with signals/slots for thread-safe UI updates.

**Rationale**: Qt signals/slots are thread-safe when using `Qt::QueuedConnection`.

**Implementation Pattern**:
```cpp
class TwitchApiWorker : public QObject {
    Q_OBJECT
signals:
    void streamsReady(QList<Streamer> streams);
    void errorOccurred(QString message);

public slots:
    void fetchStreams(FilterPreset preset);
};

// Usage in dock
QThread *workerThread = new QThread();
TwitchApiWorker *worker = new TwitchApiWorker();
worker->moveToThread(workerThread);

connect(worker, &TwitchApiWorker::streamsReady,
        this, &RaidPickerDock::onStreamsReady, Qt::QueuedConnection);
```

**Alternatives Considered**:
- std::async - Less Qt-integrated, harder to cancel
- OBS threading helpers - Less documented, Qt preferred for UI plugins

### 2.5 Build System

**Decision**: Use CMake with OBS plugin template.

**Starting Point**: `git clone --recursive https://github.com/obsproject/obs-plugintemplate.git`

**CMakeLists.txt Structure**:
```cmake
cmake_minimum_required(VERSION 3.16)
project(hoo-raid VERSION 1.0.0)

find_package(libobs REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
find_package(CURL REQUIRED)

add_library(hoo-raid MODULE
    src/plugin.cpp
    src/ui/raid-picker-dock.cpp
    # ... other sources
)

target_link_libraries(hoo-raid
    OBS::libobs
    OBS::frontend-api
    Qt6::Widgets
    Qt6::Network
    CURL::libcurl
)
```

---

## 3. Secure Credential Storage

### 3.1 Library Selection

**Decision**: Use [QtKeychain](https://github.com/frankosterfeld/qtkeychain) for OAuth token storage.

**Rationale**:
- Already using Qt 6 for UI
- Supports Qt 5 and Qt 6 with C++11
- Cross-platform: Windows Credential Store, macOS Keychain, Linux libsecret
- Active maintenance, ConanCenter package available

**Alternatives Considered**:
| Library | Pros | Cons |
|---------|------|------|
| libcred | Pure C++, lightweight | Less Qt integration |
| keychain (hrantzsch) | Simple API | Less active development |
| SafeKeeping | Modern C++ | Newer, less proven |
| Raw platform APIs | No dependency | Significant platform-specific code |

### 3.2 Platform Backends

| Platform | Backend | Notes |
|----------|---------|-------|
| Windows 7+ | Credential Vault | Encrypted with user login |
| macOS | Keychain | ACL per app, prompts for unauthorized access |
| Linux | libsecret (GNOME Keyring/KWallet) | Requires running keyring daemon |

**Linux Dependency**: `libsecret-1-dev` package required.

### 3.3 Implementation Pattern

**Decision**: Store access token and refresh token separately, re-auth on failure.

```cpp
// Write token
QKeychain::WritePasswordJob job(QLatin1String("HOO-Raid"));
job.setKey("twitch_access_token");
job.setTextData(accessToken);
job.start();

// Read token
QKeychain::ReadPasswordJob job(QLatin1String("HOO-Raid"));
job.setKey("twitch_access_token");
job.start();
// Connect to finished() signal for result
```

### 3.4 Security Considerations

- Tokens encrypted at rest by OS credential manager
- Never log tokens (mask in debug output)
- Check token validity before each session, refresh if expired
- Clear tokens on explicit logout

---

## Implementation Recommendations

### API Call Strategy

1. **Stream Discovery Flow**:
   - Fetch streams by language + game (paginated)
   - Batch-fetch follower counts for returned streams
   - Apply viewer/follower filters client-side
   - Cache results for re-roll without re-querying

2. **Caching**:
   - Cache stream list for 60 seconds (stale data acceptable for raids)
   - Cache game name → ID mappings indefinitely
   - Invalidate on preset/filter change

3. **Error Handling**:
   - Retry 429 (rate limit) with exponential backoff
   - Show user-friendly error for API failures
   - Queue raid even if network flaky (Twitch handles)

### UI Responsiveness

1. Show loading indicator immediately on action
2. Populate results as they arrive (progressive rendering)
3. Allow cancel of in-flight requests
4. Disable raid button until preview loaded

---

## Sources

- [Twitch API Reference](https://dev.twitch.tv/docs/api/reference/)
- [Twitch Raids Documentation](https://dev.twitch.tv/docs/api/raids)
- [Twitch OAuth Scopes](https://dev.twitch.tv/docs/authentication/scopes/)
- [OBS Developer Guide](https://obsproject.com/kb/developer-guide)
- [OBS Plugin Template](https://github.com/obsproject/obs-plugintemplate)
- [QtKeychain](https://github.com/frankosterfeld/qtkeychain)
- [OBS Forums - Dockable Panels](https://obsproject.com/forum/threads/obs-add-a-new-custom-panel-to-the-dock.78945/)
