#include "twitch-api.h"
#include "oauth-manager.h"
#include <obs-module.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

int RateLimitInfo::secondsUntilReset() const
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    int seconds = resetTimestamp - static_cast<int>(now);
    return seconds > 0 ? seconds : 0;
}

TwitchApi::TwitchApi(OAuthManager* oauthManager, QObject* parent)
    : QObject(parent)
    , m_oauthManager(oauthManager)
{
}

void TwitchApi::fetchStreams(
    const QString& language,
    const QString& gameId,
    int first,
    std::function<void(const ApiResponse<QList<Streamer>>&)> callback)
{
    QUrlQuery params;
    params.addQueryItem("first", QString::number(qMin(first, 100)));

    if (!language.isEmpty()) {
        params.addQueryItem("language", language);
    }
    if (!gameId.isEmpty()) {
        params.addQueryItem("game_id", gameId);
    }

    makeRequest("GET", "/streams", params,
        [callback](const QByteArray& data, TwitchApiError error, const QString& errorMsg, const RateLimitInfo& rateLimit) {
            ApiResponse<QList<Streamer>> response;
            response.error = error;
            response.errorMessage = errorMsg;
            response.rateLimit = rateLimit;

            if (error == TwitchApiError::None) {
                QJsonDocument doc = QJsonDocument::fromJson(data);
                QJsonArray dataArray = doc.object()["data"].toArray();

                for (const QJsonValue& v : dataArray) {
                    Streamer s = Streamer::fromTwitchStreamJson(v.toObject());
                    if (s.isValid()) {
                        response.data.append(s);
                    }
                }

                blog(LOG_DEBUG, "[HOO-Raid] Fetched %d streams", response.data.size());
            }

            callback(response);
        });
}

void TwitchApi::fetchFollowedStreams(
    const QString& userId,
    int first,
    std::function<void(const ApiResponse<QList<Streamer>>&)> callback)
{
    QUrlQuery params;
    params.addQueryItem("user_id", userId);
    params.addQueryItem("first", QString::number(qMin(first, 100)));

    makeRequest("GET", "/streams/followed", params,
        [callback](const QByteArray& data, TwitchApiError error, const QString& errorMsg, const RateLimitInfo& rateLimit) {
            ApiResponse<QList<Streamer>> response;
            response.error = error;
            response.errorMessage = errorMsg;
            response.rateLimit = rateLimit;

            if (error == TwitchApiError::None) {
                QJsonDocument doc = QJsonDocument::fromJson(data);
                QJsonArray dataArray = doc.object()["data"].toArray();

                for (const QJsonValue& v : dataArray) {
                    Streamer s = Streamer::fromTwitchStreamJson(v.toObject());
                    if (s.isValid()) {
                        response.data.append(s);
                    }
                }

                blog(LOG_DEBUG, "[HOO-Raid] Fetched %d followed streams", response.data.size());
            }

            callback(response);
        });
}

void TwitchApi::fetchFollowerCount(
    const QString& broadcasterId,
    std::function<void(const ApiResponse<int>&)> callback)
{
    QUrlQuery params;
    params.addQueryItem("broadcaster_id", broadcasterId);
    params.addQueryItem("first", "1"); // We only need the total count

    makeRequest("GET", "/channels/followers", params,
        [callback](const QByteArray& data, TwitchApiError error, const QString& errorMsg, const RateLimitInfo& rateLimit) {
            ApiResponse<int> response;
            response.error = error;
            response.errorMessage = errorMsg;
            response.rateLimit = rateLimit;
            response.data = 0;

            if (error == TwitchApiError::None) {
                QJsonDocument doc = QJsonDocument::fromJson(data);
                response.data = doc.object()["total"].toInt();
                blog(LOG_DEBUG, "[HOO-Raid] Fetched follower count: %d", response.data);
            }

            callback(response);
        });
}

void TwitchApi::startRaid(
    const QString& fromBroadcasterId,
    const QString& toBroadcasterId,
    std::function<void(const ApiResponse<bool>&)> callback)
{
    QUrlQuery params;
    params.addQueryItem("from_broadcaster_id", fromBroadcasterId);
    params.addQueryItem("to_broadcaster_id", toBroadcasterId);

    makeRequest("POST", "/raids", params,
        [callback](const QByteArray& data, TwitchApiError error, const QString& errorMsg, const RateLimitInfo& rateLimit) {
            Q_UNUSED(data);
            ApiResponse<bool> response;
            response.error = error;
            response.errorMessage = errorMsg;
            response.rateLimit = rateLimit;
            response.data = (error == TwitchApiError::None);

            if (response.data) {
                blog(LOG_INFO, "[HOO-Raid] Raid started successfully");
            } else {
                blog(LOG_WARNING, "[HOO-Raid] Raid failed: %s", errorMsg.toUtf8().constData());
            }

            callback(response);
        });
}

void TwitchApi::cancelRaid(
    const QString& broadcasterId,
    std::function<void(const ApiResponse<bool>&)> callback)
{
    QUrlQuery params;
    params.addQueryItem("broadcaster_id", broadcasterId);

    makeRequest("DELETE", "/raids", params,
        [callback](const QByteArray& data, TwitchApiError error, const QString& errorMsg, const RateLimitInfo& rateLimit) {
            Q_UNUSED(data);
            ApiResponse<bool> response;
            response.error = error;
            response.errorMessage = errorMsg;
            response.rateLimit = rateLimit;
            response.data = (error == TwitchApiError::None);

            if (response.data) {
                blog(LOG_INFO, "[HOO-Raid] Raid cancelled");
            }

            callback(response);
        });
}

void TwitchApi::searchGames(
    const QString& query,
    int first,
    std::function<void(const ApiResponse<QList<Game>>&)> callback)
{
    QUrlQuery params;
    params.addQueryItem("query", query);
    params.addQueryItem("first", QString::number(qMin(first, 100)));

    makeRequest("GET", "/search/categories", params,
        [callback](const QByteArray& data, TwitchApiError error, const QString& errorMsg, const RateLimitInfo& rateLimit) {
            ApiResponse<QList<Game>> response;
            response.error = error;
            response.errorMessage = errorMsg;
            response.rateLimit = rateLimit;

            if (error == TwitchApiError::None) {
                QJsonDocument doc = QJsonDocument::fromJson(data);
                QJsonArray dataArray = doc.object()["data"].toArray();

                for (const QJsonValue& v : dataArray) {
                    Game g = Game::fromTwitchJson(v.toObject());
                    if (g.isValid()) {
                        response.data.append(g);
                    }
                }

                blog(LOG_DEBUG, "[HOO-Raid] Found %d games matching '%s'",
                     response.data.size(), query.toUtf8().constData());
            }

            callback(response);
        });
}

void TwitchApi::makeRequest(
    const QString& method,
    const QString& endpoint,
    const QUrlQuery& params,
    std::function<void(const QByteArray&, TwitchApiError, const QString&, const RateLimitInfo&)> callback)
{
    if (!m_oauthManager || !m_oauthManager->isAuthenticated()) {
        blog(LOG_WARNING, "[HOO-Raid] API request failed: not authenticated");
        callback(QByteArray(), TwitchApiError::Unauthorized, "Not authenticated", RateLimitInfo());
        emit unauthorized();
        return;
    }

    // Check rate limit
    if (m_lastRateLimit.isLimited()) {
        int seconds = m_lastRateLimit.secondsUntilReset();
        if (seconds > 0) {
            blog(LOG_WARNING, "[HOO-Raid] Rate limited, %d seconds until reset", seconds);
            callback(QByteArray(), TwitchApiError::RateLimited,
                     QString("Rate limited. Retry in %1 seconds.").arg(seconds), m_lastRateLimit);
            emit rateLimited(seconds);
            return;
        }
    }

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QUrl url(QString("%1%2").arg(BASE_URL, endpoint));
    if (method == "GET") {
        url.setQuery(params);
    }

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_oauthManager->accessToken()).toUtf8());
    request.setRawHeader("Client-Id", CLIENT_ID);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = nullptr;
    if (method == "GET") {
        reply = manager->get(request);
    } else if (method == "POST") {
        QUrl postUrl(QString("%1%2").arg(BASE_URL, endpoint));
        postUrl.setQuery(params);
        request.setUrl(postUrl);
        reply = manager->post(request, QByteArray());
    } else if (method == "DELETE") {
        QUrl deleteUrl(QString("%1%2").arg(BASE_URL, endpoint));
        deleteUrl.setQuery(params);
        request.setUrl(deleteUrl);
        reply = manager->deleteResource(request);
    }

    if (!reply) {
        callback(QByteArray(), TwitchApiError::NetworkError, "Failed to create request", RateLimitInfo());
        manager->deleteLater();
        return;
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, callback]() {
        reply->deleteLater();
        manager->deleteLater();

        RateLimitInfo rateLimit = parseRateLimitHeaders(reply);
        m_lastRateLimit = rateLimit;

        if (reply->error() != QNetworkReply::NoError) {
            TwitchApiError error = mapNetworkError(reply->error());
            QString errorMsg = reply->errorString();

            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (statusCode == 401) {
                error = TwitchApiError::Unauthorized;
                emit unauthorized();
            } else if (statusCode == 429) {
                error = TwitchApiError::RateLimited;
                emit rateLimited(rateLimit.secondsUntilReset());
            } else if (statusCode == 404) {
                error = TwitchApiError::NotFound;
            } else if (statusCode >= 500) {
                error = TwitchApiError::ServerError;
            }

            blog(LOG_WARNING, "[HOO-Raid] API error: %s (status %d)",
                 errorMsg.toUtf8().constData(), statusCode);
            callback(QByteArray(), error, errorMsg, rateLimit);
            return;
        }

        QByteArray data = reply->readAll();
        callback(data, TwitchApiError::None, QString(), rateLimit);
    });
}

RateLimitInfo TwitchApi::parseRateLimitHeaders(const QNetworkReply* reply)
{
    RateLimitInfo info;
    info.remaining = reply->rawHeader("Ratelimit-Remaining").toInt();
    info.limit = reply->rawHeader("Ratelimit-Limit").toInt();
    info.resetTimestamp = reply->rawHeader("Ratelimit-Reset").toInt();
    return info;
}

TwitchApiError TwitchApi::mapNetworkError(QNetworkReply::NetworkError error)
{
    switch (error) {
    case QNetworkReply::NoError:
        return TwitchApiError::None;
    case QNetworkReply::AuthenticationRequiredError:
        return TwitchApiError::Unauthorized;
    case QNetworkReply::ContentNotFoundError:
        return TwitchApiError::NotFound;
    case QNetworkReply::InternalServerError:
    case QNetworkReply::ServiceUnavailableError:
        return TwitchApiError::ServerError;
    default:
        return TwitchApiError::NetworkError;
    }
}
