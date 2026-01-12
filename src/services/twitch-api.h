#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <functional>
#include "../models/streamer.h"
#include "../models/game.h"

class OAuthManager;

// Rate limit info from Twitch API response
struct RateLimitInfo {
    int remaining = 0;
    int limit = 0;
    int resetTimestamp = 0;
    bool isLimited() const { return remaining <= 0; }
    int secondsUntilReset() const;
};

// API error types
enum class TwitchApiError {
    None,
    NetworkError,
    Unauthorized,
    RateLimited,
    NotFound,
    ServerError,
    InvalidResponse
};

// API response wrapper
template<typename T>
struct ApiResponse {
    T data;
    TwitchApiError error = TwitchApiError::None;
    QString errorMessage;
    RateLimitInfo rateLimit;
    bool isSuccess() const { return error == TwitchApiError::None; }
};

// Interface for Twitch API
class ITwitchApi {
public:
    virtual ~ITwitchApi() = default;

    // Streams
    virtual void fetchStreams(
        const QString& language,
        const QString& gameId,
        int first,
        std::function<void(const ApiResponse<QList<Streamer>>&)> callback) = 0;

    virtual void fetchFollowedStreams(
        const QString& userId,
        int first,
        std::function<void(const ApiResponse<QList<Streamer>>&)> callback) = 0;

    // Followers
    virtual void fetchFollowerCount(
        const QString& broadcasterId,
        std::function<void(const ApiResponse<int>&)> callback) = 0;

    // Raids
    virtual void startRaid(
        const QString& fromBroadcasterId,
        const QString& toBroadcasterId,
        std::function<void(const ApiResponse<bool>&)> callback) = 0;

    virtual void cancelRaid(
        const QString& broadcasterId,
        std::function<void(const ApiResponse<bool>&)> callback) = 0;

    // Games/Categories
    virtual void searchGames(
        const QString& query,
        int first,
        std::function<void(const ApiResponse<QList<Game>>&)> callback) = 0;
};

// Twitch Helix API implementation
class TwitchApi : public QObject, public ITwitchApi {
    Q_OBJECT

public:
    explicit TwitchApi(OAuthManager* oauthManager, QObject* parent = nullptr);
    ~TwitchApi() override = default;

    // ITwitchApi interface
    void fetchStreams(
        const QString& language,
        const QString& gameId,
        int first,
        std::function<void(const ApiResponse<QList<Streamer>>&)> callback) override;

    void fetchFollowedStreams(
        const QString& userId,
        int first,
        std::function<void(const ApiResponse<QList<Streamer>>&)> callback) override;

    void fetchFollowerCount(
        const QString& broadcasterId,
        std::function<void(const ApiResponse<int>&)> callback) override;

    void startRaid(
        const QString& fromBroadcasterId,
        const QString& toBroadcasterId,
        std::function<void(const ApiResponse<bool>&)> callback) override;

    void cancelRaid(
        const QString& broadcasterId,
        std::function<void(const ApiResponse<bool>&)> callback) override;

    void searchGames(
        const QString& query,
        int first,
        std::function<void(const ApiResponse<QList<Game>>&)> callback) override;

signals:
    void rateLimited(int secondsUntilReset);
    void unauthorized();

private:
    static constexpr const char* BASE_URL = "https://api.twitch.tv/helix";
    static constexpr const char* CLIENT_ID = "YOUR_TWITCH_CLIENT_ID"; // TODO: Replace

    OAuthManager* m_oauthManager;
    RateLimitInfo m_lastRateLimit;

    // Request helpers
    void makeRequest(
        const QString& method,
        const QString& endpoint,
        const QUrlQuery& params,
        std::function<void(const QByteArray&, TwitchApiError, const QString&, const RateLimitInfo&)> callback);

    RateLimitInfo parseRateLimitHeaders(const QNetworkReply* reply);
    TwitchApiError mapNetworkError(QNetworkReply::NetworkError error);
};
