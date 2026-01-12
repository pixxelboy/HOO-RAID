#include "settings-manager.h"
#include <obs-module.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
{
}

SettingsManager::~SettingsManager()
{
    save();
}

QString SettingsManager::settingsFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath(SETTINGS_FILE);
}

void SettingsManager::load()
{
    QString path = settingsFilePath();
    QFile file(path);

    if (!file.exists()) {
        blog(LOG_DEBUG, "[HOO-Raid] No settings file found at %s", path.toUtf8().constData());
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        blog(LOG_ERROR, "[HOO-Raid] Failed to open settings file: %s",
             file.errorString().toUtf8().constData());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        blog(LOG_ERROR, "[HOO-Raid] Invalid settings file format");
        return;
    }

    QJsonObject root = doc.object();

    // Load custom presets
    m_customPresets.clear();
    QJsonArray presetsArray = root["custom_presets"].toArray();
    for (const QJsonValue& v : presetsArray) {
        FilterPreset preset = FilterPreset::fromJson(v.toObject());
        if (preset.isValid()) {
            m_customPresets.append(preset);
        }
    }

    // Load greylist
    m_greylist.clear();
    QJsonArray greylistArray = root["greylist"].toArray();
    for (const QJsonValue& v : greylistArray) {
        GreylistEntry entry = GreylistEntry::fromJson(v.toObject());
        if (entry.isValid()) {
            m_greylist.append(entry);
        }
    }

    // Load UI state
    m_lastPresetId = root["last_preset_id"].toString();

    // Load cached user info
    QJsonObject userCache = root["user_cache"].toObject();
    m_cachedUserId = userCache["user_id"].toString();
    m_cachedUserLogin = userCache["user_login"].toString();

    blog(LOG_INFO, "[HOO-Raid] Settings loaded: %d custom presets, %d greylist entries",
         m_customPresets.size(), m_greylist.size());
}

void SettingsManager::save()
{
    QString path = settingsFilePath();

    QJsonObject root;

    // Save custom presets
    QJsonArray presetsArray;
    for (const FilterPreset& preset : m_customPresets) {
        presetsArray.append(preset.toJson());
    }
    root["custom_presets"] = presetsArray;

    // Save greylist
    QJsonArray greylistArray;
    for (const GreylistEntry& entry : m_greylist) {
        greylistArray.append(entry.toJson());
    }
    root["greylist"] = greylistArray;

    // Save UI state
    root["last_preset_id"] = m_lastPresetId;

    // Save cached user info
    QJsonObject userCache;
    userCache["user_id"] = m_cachedUserId;
    userCache["user_login"] = m_cachedUserLogin;
    root["user_cache"] = userCache;

    QJsonDocument doc(root);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        blog(LOG_ERROR, "[HOO-Raid] Failed to save settings: %s",
             file.errorString().toUtf8().constData());
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    blog(LOG_DEBUG, "[HOO-Raid] Settings saved to %s", path.toUtf8().constData());
}

QList<FilterPreset> SettingsManager::customPresets() const
{
    return m_customPresets;
}

void SettingsManager::saveCustomPreset(const FilterPreset& preset)
{
    // Check if updating existing
    for (int i = 0; i < m_customPresets.size(); ++i) {
        if (m_customPresets[i].id == preset.id) {
            m_customPresets[i] = preset;
            emit settingsChanged();
            return;
        }
    }

    // Add new
    m_customPresets.append(preset);
    emit presetAdded(preset);
    emit settingsChanged();
}

void SettingsManager::deleteCustomPreset(const QString& presetId)
{
    for (int i = 0; i < m_customPresets.size(); ++i) {
        if (m_customPresets[i].id == presetId) {
            m_customPresets.removeAt(i);
            emit presetRemoved(presetId);
            emit settingsChanged();
            return;
        }
    }
}

QList<GreylistEntry> SettingsManager::greylist() const
{
    return m_greylist;
}

void SettingsManager::addToGreylist(const GreylistEntry& entry)
{
    // Check if already exists
    for (const GreylistEntry& existing : m_greylist) {
        if (existing.userId == entry.userId) {
            return; // Already greylisted
        }
    }

    m_greylist.append(entry);
    emit greylistChanged();
    emit settingsChanged();

    blog(LOG_INFO, "[HOO-Raid] Added to greylist: %s",
         entry.userName.toUtf8().constData());
}

void SettingsManager::removeFromGreylist(const QString& userId)
{
    for (int i = 0; i < m_greylist.size(); ++i) {
        if (m_greylist[i].userId == userId) {
            QString userName = m_greylist[i].userName;
            m_greylist.removeAt(i);
            emit greylistChanged();
            emit settingsChanged();

            blog(LOG_INFO, "[HOO-Raid] Removed from greylist: %s",
                 userName.toUtf8().constData());
            return;
        }
    }
}

bool SettingsManager::isGreylisted(const QString& userId) const
{
    for (const GreylistEntry& entry : m_greylist) {
        if (entry.userId == userId) {
            return true;
        }
    }
    return false;
}

QString SettingsManager::lastPresetId() const
{
    return m_lastPresetId;
}

void SettingsManager::setLastPresetId(const QString& presetId)
{
    if (m_lastPresetId != presetId) {
        m_lastPresetId = presetId;
        emit settingsChanged();
    }
}

QString SettingsManager::cachedUserId() const
{
    return m_cachedUserId;
}

QString SettingsManager::cachedUserLogin() const
{
    return m_cachedUserLogin;
}

void SettingsManager::cacheUserInfo(const QString& userId, const QString& userLogin)
{
    m_cachedUserId = userId;
    m_cachedUserLogin = userLogin;
    emit settingsChanged();
}
