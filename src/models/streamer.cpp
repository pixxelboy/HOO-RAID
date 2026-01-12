#include "streamer.h"
#include <QJsonArray>

QString Streamer::formattedDuration() const
{
    if (!startedAt.isValid()) {
        return QString();
    }

    qint64 seconds = startedAt.secsTo(QDateTime::currentDateTimeUtc());
    if (seconds < 0) {
        return QString();
    }

    int hours = static_cast<int>(seconds / 3600);
    int minutes = static_cast<int>((seconds % 3600) / 60);

    if (hours > 0) {
        return QString("%1h %2m").arg(hours).arg(minutes);
    }
    return QString("%1m").arg(minutes);
}

QString Streamer::thumbnailUrlWithSize(int width, int height) const
{
    if (thumbnailUrl.isEmpty()) {
        return QString();
    }

    QString url = thumbnailUrl;
    url.replace("{width}", QString::number(width));
    url.replace("{height}", QString::number(height));
    return url;
}

QJsonObject Streamer::toJson() const
{
    QJsonObject obj;
    obj["user_id"] = userId;
    obj["user_login"] = userLogin;
    obj["user_name"] = userName;
    obj["stream_id"] = streamId;
    obj["title"] = title;
    obj["game_id"] = gameId;
    obj["game_name"] = gameName;
    obj["language"] = language;
    obj["viewer_count"] = viewerCount;
    obj["started_at"] = startedAt.toString(Qt::ISODate);
    obj["thumbnail_url"] = thumbnailUrl;
    obj["follower_count"] = followerCount;
    return obj;
}

Streamer Streamer::fromJson(const QJsonObject& obj)
{
    Streamer s;
    s.userId = obj["user_id"].toString();
    s.userLogin = obj["user_login"].toString();
    s.userName = obj["user_name"].toString();
    s.streamId = obj["stream_id"].toString();
    s.title = obj["title"].toString();
    s.gameId = obj["game_id"].toString();
    s.gameName = obj["game_name"].toString();
    s.language = obj["language"].toString();
    s.viewerCount = obj["viewer_count"].toInt();
    s.startedAt = QDateTime::fromString(obj["started_at"].toString(), Qt::ISODate);
    s.thumbnailUrl = obj["thumbnail_url"].toString();
    s.followerCount = obj["follower_count"].toInt();
    return s;
}

Streamer Streamer::fromTwitchStreamJson(const QJsonObject& obj)
{
    Streamer s;
    s.userId = obj["user_id"].toString();
    s.userLogin = obj["user_login"].toString();
    s.userName = obj["user_name"].toString();
    s.streamId = obj["id"].toString();
    s.title = obj["title"].toString();
    s.gameId = obj["game_id"].toString();
    s.gameName = obj["game_name"].toString();
    s.language = obj["language"].toString();
    s.viewerCount = obj["viewer_count"].toInt();
    s.startedAt = QDateTime::fromString(obj["started_at"].toString(), Qt::ISODate);
    s.thumbnailUrl = obj["thumbnail_url"].toString();
    // follower_count is fetched separately via Get Channel Followers API
    s.followerCount = 0;
    return s;
}
