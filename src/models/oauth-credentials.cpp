#include "oauth-credentials.h"

bool OAuthCredentials::isExpired() const
{
    if (!expiresAt.isValid()) {
        return false; // No expiry set, assume valid
    }
    return QDateTime::currentDateTimeUtc() >= expiresAt;
}

bool OAuthCredentials::hasScope(const QString& scope) const
{
    return scopes.contains(scope);
}

bool OAuthCredentials::hasRequiredScopes() const
{
    for (const QString& required : requiredScopes()) {
        if (!hasScope(required)) {
            return false;
        }
    }
    return true;
}

qint64 OAuthCredentials::secondsUntilExpiry() const
{
    if (!expiresAt.isValid()) {
        return -1; // No expiry
    }
    return QDateTime::currentDateTimeUtc().secsTo(expiresAt);
}

QJsonObject OAuthCredentials::toJson() const
{
    // Note: Does NOT include accessToken or refreshToken for security
    // Those are stored separately in the credential store
    QJsonObject obj;
    obj["expires_at"] = expiresAt.toString(Qt::ISODate);
    obj["user_id"] = userId;
    obj["user_login"] = userLogin;
    obj["user_name"] = userName;

    QJsonArray scopeArray;
    for (const QString& scope : scopes) {
        scopeArray.append(scope);
    }
    obj["scopes"] = scopeArray;

    return obj;
}

OAuthCredentials OAuthCredentials::fromJson(const QJsonObject& obj)
{
    OAuthCredentials c;
    c.expiresAt = QDateTime::fromString(obj["expires_at"].toString(), Qt::ISODate);
    c.userId = obj["user_id"].toString();
    c.userLogin = obj["user_login"].toString();
    c.userName = obj["user_name"].toString();

    QJsonArray scopeArray = obj["scopes"].toArray();
    for (const QJsonValue& v : scopeArray) {
        c.scopes.append(v.toString());
    }

    return c;
}

QStringList OAuthCredentials::requiredScopes()
{
    return {
        "user:read:follows",
        "channel:manage:raids"
    };
}
