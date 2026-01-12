#pragma once

#include <QObject>
#include <QList>
#include "../models/streamer.h"
#include "../models/filter-preset.h"

class SettingsManager;

// Interface for streamer selection
class IStreamerSelector {
public:
    virtual ~IStreamerSelector() = default;

    virtual void setPool(const QList<Streamer>& streamers) = 0;
    virtual void setFilter(const FilterPreset& preset) = 0;
    virtual Streamer selectRandom() = 0;
    virtual Streamer selectNext() = 0;
    virtual bool hasMore() const = 0;
    virtual int poolSize() const = 0;
    virtual int remainingCount() const = 0;
    virtual void excludeFromPool(const QString& userId) = 0;
    virtual void reset() = 0;
};

// Fisher-Yates shuffle based streamer selector
class StreamerSelector : public QObject, public IStreamerSelector {
    Q_OBJECT

public:
    explicit StreamerSelector(SettingsManager* settingsManager = nullptr, QObject* parent = nullptr);
    ~StreamerSelector() override = default;

    // IStreamerSelector interface
    void setPool(const QList<Streamer>& streamers) override;
    void setFilter(const FilterPreset& preset) override;
    Streamer selectRandom() override;
    Streamer selectNext() override;
    bool hasMore() const override;
    int poolSize() const override;
    int remainingCount() const override;
    void excludeFromPool(const QString& userId) override;
    void reset() override;

signals:
    void poolUpdated(int size);
    void streamerSelected(const Streamer& streamer);
    void poolExhausted();

private:
    void shufflePool();
    void applyFiltersAndGreylist();

    SettingsManager* m_settingsManager;
    QList<Streamer> m_originalPool;
    QList<Streamer> m_filteredPool;
    QList<Streamer> m_shuffledPool;
    FilterPreset m_currentFilter;
    int m_currentIndex = 0;
};
