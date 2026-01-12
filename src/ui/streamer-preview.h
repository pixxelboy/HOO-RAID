#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "../models/streamer.h"

class StreamerPreview : public QWidget {
    Q_OBJECT

public:
    explicit StreamerPreview(QWidget* parent = nullptr);
    ~StreamerPreview() override = default;

    void setStreamer(const Streamer& streamer);
    void clear();

    const Streamer& streamer() const { return m_streamer; }

signals:
    void pickAnotherClicked();
    void launchRaidClicked();
    void addToGreylistClicked();

private:
    void setupUi();
    void updateDisplay();

    // UI Components
    QVBoxLayout* m_layout = nullptr;
    QLabel* m_thumbnailLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_categoryLabel = nullptr;
    QLabel* m_viewersLabel = nullptr;
    QLabel* m_followersLabel = nullptr;
    QLabel* m_durationLabel = nullptr;

    // Action buttons
    QPushButton* m_pickAnotherButton = nullptr;
    QPushButton* m_launchRaidButton = nullptr;
    QPushButton* m_greylistButton = nullptr;

    // State
    Streamer m_streamer;
};
