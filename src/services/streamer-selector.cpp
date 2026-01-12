#include "streamer-selector.h"
#include "../storage/settings-manager.h"
#include <obs-module.h>
#include <QRandomGenerator>
#include <algorithm>

StreamerSelector::StreamerSelector(SettingsManager* settingsManager, QObject* parent)
    : QObject(parent)
    , m_settingsManager(settingsManager)
{
}

void StreamerSelector::setPool(const QList<Streamer>& streamers)
{
    m_originalPool = streamers;
    applyFiltersAndGreylist();
    shufflePool();
    m_currentIndex = 0;

    blog(LOG_DEBUG, "[HOO-Raid] Pool set with %d streamers, %d after filtering",
         m_originalPool.size(), m_shuffledPool.size());

    emit poolUpdated(m_shuffledPool.size());
}

void StreamerSelector::setFilter(const FilterPreset& preset)
{
    m_currentFilter = preset;
    applyFiltersAndGreylist();
    shufflePool();
    m_currentIndex = 0;

    blog(LOG_DEBUG, "[HOO-Raid] Filter applied: %s, %d streamers remaining",
         preset.name.toUtf8().constData(), m_shuffledPool.size());

    emit poolUpdated(m_shuffledPool.size());
}

Streamer StreamerSelector::selectRandom()
{
    if (m_shuffledPool.isEmpty()) {
        blog(LOG_DEBUG, "[HOO-Raid] No streamers in pool");
        return Streamer();
    }

    // Reshuffle and start from beginning
    shufflePool();
    m_currentIndex = 0;

    return selectNext();
}

Streamer StreamerSelector::selectNext()
{
    if (m_currentIndex >= m_shuffledPool.size()) {
        blog(LOG_DEBUG, "[HOO-Raid] Pool exhausted");
        emit poolExhausted();
        return Streamer();
    }

    Streamer selected = m_shuffledPool[m_currentIndex];
    m_currentIndex++;

    blog(LOG_DEBUG, "[HOO-Raid] Selected streamer: %s (%d/%d)",
         selected.userName.toUtf8().constData(),
         m_currentIndex, m_shuffledPool.size());

    emit streamerSelected(selected);
    return selected;
}

bool StreamerSelector::hasMore() const
{
    return m_currentIndex < m_shuffledPool.size();
}

int StreamerSelector::poolSize() const
{
    return m_shuffledPool.size();
}

int StreamerSelector::remainingCount() const
{
    return m_shuffledPool.size() - m_currentIndex;
}

void StreamerSelector::excludeFromPool(const QString& userId)
{
    // Remove from all pools
    auto removeById = [&userId](QList<Streamer>& list) {
        list.erase(std::remove_if(list.begin(), list.end(),
            [&userId](const Streamer& s) { return s.userId == userId; }),
            list.end());
    };

    removeById(m_originalPool);
    removeById(m_filteredPool);
    removeById(m_shuffledPool);

    // Adjust current index if needed
    if (m_currentIndex > m_shuffledPool.size()) {
        m_currentIndex = m_shuffledPool.size();
    }

    blog(LOG_DEBUG, "[HOO-Raid] Excluded user %s from pool, %d remaining",
         userId.toUtf8().constData(), m_shuffledPool.size());

    emit poolUpdated(m_shuffledPool.size());
}

void StreamerSelector::reset()
{
    m_originalPool.clear();
    m_filteredPool.clear();
    m_shuffledPool.clear();
    m_currentIndex = 0;
    emit poolUpdated(0);
}

void StreamerSelector::shufflePool()
{
    m_shuffledPool = m_filteredPool;

    // Fisher-Yates shuffle
    int n = m_shuffledPool.size();
    for (int i = n - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        m_shuffledPool.swapItemsAt(i, j);
    }
}

void StreamerSelector::applyFiltersAndGreylist()
{
    m_filteredPool.clear();

    for (const Streamer& streamer : m_originalPool) {
        // Check greylist
        if (m_settingsManager && m_settingsManager->isGreylisted(streamer.userId)) {
            continue;
        }

        // Check filter preset
        if (m_currentFilter.isValid() && !m_currentFilter.matches(streamer)) {
            continue;
        }

        m_filteredPool.append(streamer);
    }
}
