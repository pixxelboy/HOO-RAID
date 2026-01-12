#pragma once

#include <QString>
#include <QJsonObject>
#include <QList>

struct Streamer;

struct FilterPreset {
    // Identity
    QString id;
    QString name;
    bool isBuiltIn = false;

    // Filter criteria
    QString language;       // Empty = any language
    QString gameId;         // Empty = any game
    QString gameName;       // For display only
    int viewersMin = 0;
    int viewersMax = 0;     // 0 = no limit
    int followersMin = 0;
    int followersMax = 0;   // 0 = no limit

    // Validation
    bool isValid() const;

    // Matching
    bool matches(const Streamer& streamer) const;
    bool matchesViewerCount(int viewers) const;
    bool matchesFollowerCount(int followers) const;

    // Serialization
    QJsonObject toJson() const;
    static FilterPreset fromJson(const QJsonObject& obj);

    // Built-in presets
    static FilterPreset beginner();
    static FilterPreset small();
    static FilterPreset medium();
    static QList<FilterPreset> builtInPresets();
};
