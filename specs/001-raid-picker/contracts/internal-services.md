# Internal Service Contracts

**Feature**: 001-raid-picker
**Date**: 2026-01-11

## Overview

This document defines the internal service interfaces for the HOO-Raid plugin. These are C++ class interfaces that define how components interact.

---

## 1. ITwitchApi

Interface for Twitch API operations.

```cpp
class ITwitchApi : public QObject {
    Q_OBJECT

public:
    virtual ~ITwitchApi() = default;

    // Stream discovery
    virtual void fetchStreams(const FilterPreset& filter, int limit = 100) = 0;
    virtual void fetchFollowedStreams(int limit = 100) = 0;
    virtual void fetchFollowerCount(const QString& broadcasterId) = 0;

    // Raid operations
    virtual void startRaid(const QString& toBroadcasterId) = 0;
    virtual void cancelRaid() = 0;

    // Game lookup
    virtual void searchGames(const QString& query) = 0;

signals:
    void streamsReady(const QList<Streamer>& streams);
    void followerCountReady(const QString& broadcasterId, int count);
    void raidStarted(const QDateTime& createdAt);
    void raidCancelled();
    void gamesFound(const QList<Game>& games);

    void errorOccurred(const QString& operation, const QString& message);
    void rateLimitHit(int retryAfterSeconds);
};
```

**Implementation Notes**:
- All methods are async (fire signals on completion)
- Token refresh handled internally
- Rate limit errors emit `rateLimitHit` for UI handling

---

## 2. IOAuthManager

Interface for OAuth credential management.

```cpp
class IOAuthManager : public QObject {
    Q_OBJECT

public:
    virtual ~IOAuthManager() = default;

    // Authentication flow
    virtual void startAuthFlow() = 0;
    virtual void handleCallback(const QString& code) = 0;
    virtual void logout() = 0;

    // Token access
    virtual bool isAuthenticated() const = 0;
    virtual QString getAccessToken() const = 0;
    virtual QString getUserId() const = 0;
    virtual QString getUserLogin() const = 0;

    // Token management
    virtual void refreshToken() = 0;
    virtual bool hasRequiredScopes() const = 0;

signals:
    void authenticationStarted(const QUrl& authUrl);
    void authenticated(const OAuthCredentials& credentials);
    void authenticationFailed(const QString& error);
    void tokenRefreshed();
    void loggedOut();
};
```

**Required Scopes Check**:
```cpp
bool hasRequiredScopes() const {
    return credentials.hasScope("user:read:follows") &&
           credentials.hasScope("channel:manage:raids");
}
```

---

## 3. ISettingsManager

Interface for persistent storage.

```cpp
class ISettingsManager : public QObject {
    Q_OBJECT

public:
    virtual ~ISettingsManager() = default;

    // Presets
    virtual QList<FilterPreset> getCustomPresets() const = 0;
    virtual void saveCustomPreset(const FilterPreset& preset) = 0;
    virtual void deleteCustomPreset(const QString& id) = 0;

    // Greylist
    virtual QList<GreylistEntry> getGreylist() const = 0;
    virtual void addToGreylist(const GreylistEntry& entry) = 0;
    virtual void removeFromGreylist(const QString& userId) = 0;
    virtual bool isGreylisted(const QString& userId) const = 0;

    // User info (cached)
    virtual void setUserInfo(const QString& userId, const QString& userLogin) = 0;
    virtual QString getCachedUserId() const = 0;
    virtual QString getCachedUserLogin() const = 0;

    // UI state
    virtual QString getLastPresetId() const = 0;
    virtual void setLastPresetId(const QString& id) = 0;

signals:
    void presetAdded(const FilterPreset& preset);
    void presetDeleted(const QString& id);
    void greylistUpdated();
};
```

---

## 4. ICredentialStore

Interface for secure credential storage.

```cpp
class ICredentialStore {
public:
    virtual ~ICredentialStore() = default;

    virtual bool storeAccessToken(const QString& token) = 0;
    virtual bool storeRefreshToken(const QString& token) = 0;

    virtual QString retrieveAccessToken() const = 0;
    virtual QString retrieveRefreshToken() const = 0;

    virtual bool clearCredentials() = 0;
    virtual bool hasStoredCredentials() const = 0;
};
```

**Implementation**: Uses QtKeychain for platform-specific secure storage.

---

## 5. IStreamerSelector

Interface for random selection logic.

```cpp
class IStreamerSelector {
public:
    virtual ~IStreamerSelector() = default;

    // Set pool of candidates (already filtered)
    virtual void setPool(const QList<Streamer>& streamers) = 0;

    // Random selection
    virtual Streamer selectRandom() = 0;
    virtual Streamer selectNext() = 0;  // For re-roll
    virtual bool hasMore() const = 0;

    // Pool management
    virtual void excludeFromPool(const QString& userId) = 0;  // For greylist
    virtual int poolSize() const = 0;
    virtual void clearPool() = 0;
};
```

**Selection Algorithm**:
- Fisher-Yates shuffle on initial pool load
- Sequential iteration through shuffled list for re-rolls
- Re-query API when pool exhausted

---

## 6. IRaidPickerDock (UI Contract)

Interface for the main dock widget.

```cpp
class IRaidPickerDock {
public:
    virtual ~IRaidPickerDock() = default;

    // State display
    virtual void showLoading(const QString& message) = 0;
    virtual void showError(const QString& message) = 0;
    virtual void showEmpty(const QString& message, const QStringList& suggestions) = 0;
    virtual void showPreview(const Streamer& streamer) = 0;
    virtual void showRaidPending(const Streamer& target, int secondsRemaining) = 0;

    // Authentication UI
    virtual void showUnauthenticated() = 0;
    virtual void showAuthenticated(const QString& userLogin) = 0;

    // Preset selector
    virtual void setPresets(const QList<FilterPreset>& presets, const QString& selectedId) = 0;
};
```

---

## Service Dependencies

```
┌─────────────────────────────────────────────────────────────┐
│                     RaidPickerDock (UI)                     │
└─────────────────────────────────────────────────────────────┘
        │              │              │              │
        ▼              ▼              ▼              ▼
┌───────────┐  ┌───────────┐  ┌───────────┐  ┌───────────┐
│ITwitchApi │  │IOAuthMgr  │  │ISettings  │  │ISelector  │
└─────┬─────┘  └─────┬─────┘  └─────┬─────┘  └───────────┘
      │              │              │
      │              ▼              │
      │       ┌───────────┐        │
      │       │ICredStore │        │
      │       └───────────┘        │
      │                            │
      └──────────┬─────────────────┘
                 ▼
         ┌─────────────┐
         │  OBS APIs   │
         │ (obs_data)  │
         └─────────────┘
```

---

## Error Handling Contract

All services MUST:

1. **Never throw exceptions** - Use signals/return values for errors
2. **Provide actionable error messages** - Include what user can do
3. **Log errors** - Using `blog()` OBS logging function
4. **Maintain consistent state** - Failed operations don't corrupt state

**Error Message Format**:
```cpp
struct ServiceError {
    QString operation;    // e.g., "Fetching streams"
    QString message;      // e.g., "Network unavailable"
    QString suggestion;   // e.g., "Check your internet connection"
    bool isRetryable;     // Can user retry?
};
```

---

## Thread Safety Contract

| Service | Thread Model |
|---------|--------------|
| ITwitchApi | Worker thread, signals to main |
| IOAuthManager | Main thread, async network calls |
| ISettingsManager | Main thread only |
| ICredentialStore | Main thread only (QtKeychain handles async) |
| IStreamerSelector | Main thread only |
| UI Components | Main thread only |

**Rule**: All signals to UI components use `Qt::QueuedConnection` to ensure main thread execution.
