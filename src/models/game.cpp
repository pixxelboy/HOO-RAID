#include "game.h"

QString Game::boxArtUrlWithSize(int width, int height) const
{
    if (boxArtUrl.isEmpty()) {
        return QString();
    }

    QString url = boxArtUrl;
    url.replace("{width}", QString::number(width));
    url.replace("{height}", QString::number(height));
    return url;
}

QJsonObject Game::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name;
    obj["box_art_url"] = boxArtUrl;
    return obj;
}

Game Game::fromJson(const QJsonObject& obj)
{
    Game g;
    g.id = obj["id"].toString();
    g.name = obj["name"].toString();
    g.boxArtUrl = obj["box_art_url"].toString();
    return g;
}

Game Game::fromTwitchJson(const QJsonObject& obj)
{
    Game g;
    g.id = obj["id"].toString();
    g.name = obj["name"].toString();
    g.boxArtUrl = obj["box_art_url"].toString();
    return g;
}
