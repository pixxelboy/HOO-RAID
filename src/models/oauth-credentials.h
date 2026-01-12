#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>

struct OAuthCredentials {
    QString accessToken;
    QString refreshToken;
    QDateTime expiresAt;
    QStringList scopes;

    // User info (cached after validation)
    QString userId;
    QString userLogin;
    QString userName;

    // Validation
    bool isValid() const { return !accessToken.isEmpty(); }
    bool isExpired() const;
    bool hasScope(const QString& scope) const;
    bool hasRequiredScopes() const;

    // Time until expiry
    qint64 secondsUntilExpiry() const;

    // Serialization (excludes tokens for security)
    QJsonObject toJson() const;
    static OAuthCredentials fromJson(const QJsonObject& obj);

    // Required scopes for this plugin
    static QStringList requiredScopes();
};
