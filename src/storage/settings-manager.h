#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include "../models/filter-preset.h"
#include "../models/greylist-entry.h"

// Interface for settings management
class ISettingsManager {
public:
    virtual ~ISettingsManager() = default;

    // Persistence
    virtual void load() = 0;
    virtual void save() = 0;

    // Custom presets
    virtual QList<FilterPreset> customPresets() const = 0;
    virtual void saveCustomPreset(const FilterPreset& preset) = 0;
    virtual void deleteCustomPreset(const QString& presetId) = 0;

    // Greylist
    virtual QList<GreylistEntry> greylist() const = 0;
    virtual void addToGreylist(const GreylistEntry& entry) = 0;
    virtual void removeFromGreylist(const QString& userId) = 0;
    virtual bool isGreylisted(const QString& userId) const = 0;

    // UI state
    virtual QString lastPresetId() const = 0;
    virtual void setLastPresetId(const QString& presetId) = 0;

    // User info cache
    virtual QString cachedUserId() const = 0;
    virtual QString cachedUserLogin() const = 0;
    virtual void cacheUserInfo(const QString& userId, const QString& userLogin) = 0;
};

// OBS data-based implementation
class SettingsManager : public QObject, public ISettingsManager {
    Q_OBJECT

public:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() override;

    // ISettingsManager interface
    void load() override;
    void save() override;

    QList<FilterPreset> customPresets() const override;
    void saveCustomPreset(const FilterPreset& preset) override;
    void deleteCustomPreset(const QString& presetId) override;

    QList<GreylistEntry> greylist() const override;
    void addToGreylist(const GreylistEntry& entry) override;
    void removeFromGreylist(const QString& userId) override;
    bool isGreylisted(const QString& userId) const override;

    QString lastPresetId() const override;
    void setLastPresetId(const QString& presetId) override;

    QString cachedUserId() const override;
    QString cachedUserLogin() const override;
    void cacheUserInfo(const QString& userId, const QString& userLogin) override;

signals:
    void settingsChanged();
    void presetAdded(const FilterPreset& preset);
    void presetRemoved(const QString& presetId);
    void greylistChanged();

private:
    static constexpr const char* SETTINGS_FILE = "hoo-raid-settings.json";

    QString settingsFilePath() const;

    QList<FilterPreset> m_customPresets;
    QList<GreylistEntry> m_greylist;
    QString m_lastPresetId;
    QString m_cachedUserId;
    QString m_cachedUserLogin;
};
