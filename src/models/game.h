#pragma once

#include <QString>
#include <QJsonObject>

struct Game {
    QString id;
    QString name;
    QString boxArtUrl;

    // Validation
    bool isValid() const { return !id.isEmpty(); }

    // Formatting
    QString boxArtUrlWithSize(int width, int height) const;

    // Serialization
    QJsonObject toJson() const;
    static Game fromJson(const QJsonObject& obj);
    static Game fromTwitchJson(const QJsonObject& obj);
};
