#include "greylist-entry.h"

QJsonObject GreylistEntry::toJson() const
{
    QJsonObject obj;
    obj["user_id"] = userId;
    obj["user_login"] = userLogin;
    obj["user_name"] = userName;
    obj["added_at"] = addedAt.toString(Qt::ISODate);
    if (!reason.isEmpty()) {
        obj["reason"] = reason;
    }
    return obj;
}

GreylistEntry GreylistEntry::fromJson(const QJsonObject& obj)
{
    GreylistEntry e;
    e.userId = obj["user_id"].toString();
    e.userLogin = obj["user_login"].toString();
    e.userName = obj["user_name"].toString();
    e.addedAt = QDateTime::fromString(obj["added_at"].toString(), Qt::ISODate);
    e.reason = obj["reason"].toString();
    return e;
}

GreylistEntry GreylistEntry::fromStreamer(const QString& userId,
                                           const QString& userLogin,
                                           const QString& userName,
                                           const QString& reason)
{
    GreylistEntry e;
    e.userId = userId;
    e.userLogin = userLogin;
    e.userName = userName;
    e.addedAt = QDateTime::currentDateTimeUtc();
    e.reason = reason;
    return e;
}
