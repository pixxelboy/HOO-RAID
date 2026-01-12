#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>

struct GreylistEntry {
    QString userId;
    QString userLogin;
    QString userName;
    QDateTime addedAt;
    QString reason; // Optional reason for greylisting

    // Validation
    bool isValid() const { return !userId.isEmpty(); }

    // Serialization
    QJsonObject toJson() const;
    static GreylistEntry fromJson(const QJsonObject& obj);

    // Factory
    static GreylistEntry fromStreamer(const QString& userId,
                                       const QString& userLogin,
                                       const QString& userName,
                                       const QString& reason = QString());
};
