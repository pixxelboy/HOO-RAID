#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>

struct Streamer {
    // Identity (from Twitch API)
    QString userId;
    QString userLogin;
    QString userName;

    // Stream info (from Get Streams)
    QString streamId;
    QString title;
    QString gameId;
    QString gameName;
    QString language;
    int viewerCount = 0;
    QDateTime startedAt;
    QString thumbnailUrl;

    // Additional data (from Get Channel Followers)
    int followerCount = 0;

    // Validation
    bool isValid() const { return !userId.isEmpty(); }

    // Formatting
    QString formattedDuration() const;
    QString thumbnailUrlWithSize(int width, int height) const;

    // Serialization
    QJsonObject toJson() const;
    static Streamer fromJson(const QJsonObject& obj);
    static Streamer fromTwitchStreamJson(const QJsonObject& obj);
};
