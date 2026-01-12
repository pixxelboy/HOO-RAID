#include "filter-preset.h"
#include "streamer.h"
#include <QUuid>

bool FilterPreset::isValid() const
{
    // Must have an ID and name
    if (id.isEmpty() || name.isEmpty()) {
        return false;
    }

    // Min cannot exceed max (when max is set)
    if (viewersMax > 0 && viewersMin > viewersMax) {
        return false;
    }
    if (followersMax > 0 && followersMin > followersMax) {
        return false;
    }

    return true;
}

bool FilterPreset::matches(const Streamer& streamer) const
{
    // Check language filter
    if (!language.isEmpty() && streamer.language != language) {
        return false;
    }

    // Check game filter
    if (!gameId.isEmpty() && streamer.gameId != gameId) {
        return false;
    }

    // Check viewer count
    if (!matchesViewerCount(streamer.viewerCount)) {
        return false;
    }

    // Check follower count
    if (!matchesFollowerCount(streamer.followerCount)) {
        return false;
    }

    return true;
}

bool FilterPreset::matchesViewerCount(int viewers) const
{
    if (viewers < viewersMin) {
        return false;
    }
    if (viewersMax > 0 && viewers > viewersMax) {
        return false;
    }
    return true;
}

bool FilterPreset::matchesFollowerCount(int followers) const
{
    if (followers < followersMin) {
        return false;
    }
    if (followersMax > 0 && followers > followersMax) {
        return false;
    }
    return true;
}

QJsonObject FilterPreset::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name;
    obj["is_built_in"] = isBuiltIn;
    obj["language"] = language;
    obj["game_id"] = gameId;
    obj["game_name"] = gameName;
    obj["viewers_min"] = viewersMin;
    obj["viewers_max"] = viewersMax;
    obj["followers_min"] = followersMin;
    obj["followers_max"] = followersMax;
    return obj;
}

FilterPreset FilterPreset::fromJson(const QJsonObject& obj)
{
    FilterPreset p;
    p.id = obj["id"].toString();
    p.name = obj["name"].toString();
    p.isBuiltIn = obj["is_built_in"].toBool();
    p.language = obj["language"].toString();
    p.gameId = obj["game_id"].toString();
    p.gameName = obj["game_name"].toString();
    p.viewersMin = obj["viewers_min"].toInt();
    p.viewersMax = obj["viewers_max"].toInt();
    p.followersMin = obj["followers_min"].toInt();
    p.followersMax = obj["followers_max"].toInt();
    return p;
}

FilterPreset FilterPreset::beginner()
{
    FilterPreset p;
    p.id = "builtin_beginner";
    p.name = "Beginner";
    p.isBuiltIn = true;
    p.viewersMin = 1;
    p.viewersMax = 5;
    p.followersMin = 0;
    p.followersMax = 100;
    return p;
}

FilterPreset FilterPreset::small()
{
    FilterPreset p;
    p.id = "builtin_small";
    p.name = "Small";
    p.isBuiltIn = true;
    p.viewersMin = 5;
    p.viewersMax = 25;
    p.followersMin = 100;
    p.followersMax = 1000;
    return p;
}

FilterPreset FilterPreset::medium()
{
    FilterPreset p;
    p.id = "builtin_medium";
    p.name = "Medium";
    p.isBuiltIn = true;
    p.viewersMin = 25;
    p.viewersMax = 100;
    p.followersMin = 1000;
    p.followersMax = 10000;
    return p;
}

QList<FilterPreset> FilterPreset::builtInPresets()
{
    return { beginner(), small(), medium() };
}
