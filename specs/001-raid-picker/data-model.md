# Data Model: Raid Picker Plugin

**Feature**: 001-raid-picker
**Date**: 2026-01-11

## Overview

This document defines the data structures for the HOO-Raid plugin. All models are designed for C++17 with Qt integration.

---

## Core Entities

### 1. Streamer

Represents a Twitch user who is currently live streaming.

```cpp
struct Streamer {
    // Identity (from Twitch API)
    QString userId;           // Twitch user ID
    QString userLogin;        // Lowercase login name
    QString userName;         // Display name (may include caps)

    // Stream info (from Get Streams)
    QString streamId;         // Current stream ID
    QString title;            // Stream title
    QString gameId;           // Current game/category ID
    QString gameName;         // Current game/category name
    QString language;         // ISO 639-1 code (e.g., "en", "fr")
    int viewerCount;          // Current viewer count
    QDateTime startedAt;      // Stream start time
    QString thumbnailUrl;     // Stream thumbnail (template URL)

    // Additional data (from Get Channel Followers)
    int followerCount;        // Total followers (separate API call)

    // Computed
    bool isValid() const { return !userId.isEmpty(); }
    QString formattedDuration() const;  // "2h 15m" format
};
```

**Source Mapping**:
| Field | Twitch API Field | Endpoint |
|-------|------------------|----------|
| userId | user_id | /helix/streams |
| userLogin | user_login | /helix/streams |
| userName | user_name | /helix/streams |
| streamId | id | /helix/streams |
| title | title | /helix/streams |
| gameId | game_id | /helix/streams |
| gameName | game_name | /helix/streams |
| language | language | /helix/streams |
| viewerCount | viewer_count | /helix/streams |
| startedAt | started_at | /helix/streams |
| thumbnailUrl | thumbnail_url | /helix/streams |
| followerCount | total | /helix/channels/followers |

---

### 2. FilterPreset

A named configuration of filter criteria for streamer discovery.

```cpp
struct FilterPreset {
    // Identity
    QString id;               // UUID for custom presets, "beginner"/"small"/"medium" for built-in
    QString name;             // Display name
    bool isBuiltIn;           // true for Beginner/Small/Medium

    // Filter criteria
    QString language;         // ISO 639-1 code, empty = any
    QString gameId;           // Game ID, empty = any
    QString gameName;         // For display (not used in filtering)

    // Ranges (inclusive, -1 = no limit)
    int minViewers;           // Minimum viewer count
    int maxViewers;           // Maximum viewer count
    int minFollowers;         // Minimum follower count
    int maxFollowers;         // Maximum follower count

    // Methods
    bool matches(const Streamer& s) const;
    QJsonObject toJson() const;
    static FilterPreset fromJson(const QJsonObject& obj);
};
```

**Built-in Presets** (from spec assumptions):

| Preset | minFollowers | maxFollowers | minViewers | maxViewers |
|--------|--------------|--------------|------------|------------|
| Beginner | 0 | 100 | 1 | 5 |
| Small | 100 | 1000 | 5 | 25 |
| Medium | 1000 | 10000 | 25 | 100 |

**Validation Rules**:
- `name` must be non-empty and unique among user presets
- `minViewers` <= `maxViewers` (when both set)
- `minFollowers` <= `maxFollowers` (when both set)
- At least one filter criterion should be set (not all empty/unlimited)

---

### 3. GreylistEntry

A record of a streamer the user has chosen to exclude from raid suggestions.

```cpp
struct GreylistEntry {
    QString userId;           // Twitch user ID (primary key)
    QString userName;         // Display name (for UI)
    QDateTime addedAt;        // When added to greylist

    QJsonObject toJson() const;
    static GreylistEntry fromJson(const QJsonObject& obj);
};
```

**Storage**: Persisted as JSON array in OBS settings.

**Validation Rules**:
- `userId` must be non-empty and unique in greylist
- `userName` stored for display (may become stale, acceptable)

---

### 4. OAuthCredentials

OAuth 2.0 credentials for Twitch API access.

```cpp
struct OAuthCredentials {
    QString accessToken;      // Bearer token for API calls
    QString refreshToken;     // Token for refreshing access
    QDateTime expiresAt;      // When access token expires
    QStringList scopes;       // Granted scopes

    QString userId;           // Authenticated user's Twitch ID
    QString userLogin;        // Authenticated user's login name

    bool isValid() const;
    bool isExpired() const;
    bool hasScope(const QString& scope) const;
};
```

**Required Scopes**:
- `user:read:follows` - For followed streams
- `channel:manage:raids` - For starting/cancelling raids

**Storage**: Access token and refresh token stored in OS credential manager (QtKeychain). User info cached in OBS settings.

---

### 5. SessionState

Runtime state for the plugin (not persisted).

```cpp
struct SessionState {
    // Authentication
    OAuthCredentials credentials;
    bool isAuthenticated;

    // Current operation
    FilterPreset activePreset;
    QList<Streamer> currentPool;      // Filtered streamers from last search
    int currentPoolIndex;             // For re-roll without re-query
    Streamer selectedStreamer;        // Currently previewed streamer

    // UI state
    bool isLoading;
    QString errorMessage;

    // Raid state
    bool raidPending;                 // Raid queued but not executed
    QDateTime raidQueuedAt;           // When raid was queued
};
```

---

## Persistence Schema

### OBS Settings JSON

```json
{
    "version": 1,
    "user": {
        "userId": "123456789",
        "userLogin": "streamer_name"
    },
    "customPresets": [
        {
            "id": "uuid-here",
            "name": "My French Preset",
            "language": "fr",
            "gameId": "",
            "gameName": "",
            "minViewers": 10,
            "maxViewers": 50,
            "minFollowers": 500,
            "maxFollowers": 5000
        }
    ],
    "greylist": [
        {
            "userId": "987654321",
            "userName": "blocked_user",
            "addedAt": "2026-01-11T10:30:00Z"
        }
    ],
    "ui": {
        "lastPresetId": "small",
        "dockGeometry": "base64-encoded-qt-geometry"
    }
}
```

**Migration**: `version` field allows schema evolution. Increment and add migration code when structure changes.

---

## State Transitions

### Authentication Flow

```
[Unauthenticated] ---(click Login)---> [OAuth Flow]
[OAuth Flow] ---(success)---> [Authenticated]
[OAuth Flow] ---(error)---> [Unauthenticated] + error message
[Authenticated] ---(token expired)---> [Refreshing]
[Refreshing] ---(success)---> [Authenticated]
[Refreshing] ---(error)---> [Unauthenticated] + prompt re-auth
[Authenticated] ---(click Logout)---> [Unauthenticated]
```

### Raid Selection Flow

```
[Idle] ---(select preset + click Pick)---> [Loading]
[Loading] ---(streams found)---> [Preview]
[Loading] ---(no matches)---> [Empty] + suggestions
[Loading] ---(error)---> [Error] + retry option
[Preview] ---(click Pick Another)---> [Preview] (next in pool or re-query)
[Preview] ---(click Launch Raid)---> [Raid Pending]
[Raid Pending] ---(90s countdown)---> [Raid Executed] or [Cancelled]
[Preview] ---(click Greylist)---> [Preview] (remove from pool, show next)
```

---

## Relationships

```
┌─────────────────┐
│  SessionState   │
└────────┬────────┘
         │ has
         ▼
┌─────────────────┐     matches      ┌─────────────────┐
│  FilterPreset   │────────────────▶│    Streamer     │
└─────────────────┘                  └────────┬────────┘
                                              │
                                              │ excluded by
                                              ▼
                                     ┌─────────────────┐
                                     │ GreylistEntry   │
                                     └─────────────────┘

┌─────────────────┐
│OAuthCredentials │◀──── stored in OS keychain
└─────────────────┘
```

---

## Indexes & Lookups

| Operation | Key | Data Structure |
|-----------|-----|----------------|
| Find preset by ID | `FilterPreset.id` | `QMap<QString, FilterPreset>` |
| Check greylist | `GreylistEntry.userId` | `QSet<QString>` for O(1) lookup |
| Re-roll streamer | `currentPoolIndex` | `QList<Streamer>` with index |

---

## Validation Summary

| Entity | Rule | Error Message |
|--------|------|---------------|
| FilterPreset | name non-empty | "Preset name is required" |
| FilterPreset | name unique | "A preset with this name already exists" |
| FilterPreset | min <= max (viewers) | "Minimum viewers cannot exceed maximum" |
| FilterPreset | min <= max (followers) | "Minimum followers cannot exceed maximum" |
| GreylistEntry | userId non-empty | "Invalid streamer" |
| GreylistEntry | userId unique | (silently ignore duplicate add) |
| OAuthCredentials | required scopes | "Missing required permissions. Please re-authenticate." |
