#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

class PresetSelector;
class StreamerPreview;
class FilterEditor;
class GreylistManager;
class OAuthManager;
class TwitchApi;
class StreamerSelector;
struct Streamer;
struct FilterPreset;

class RaidPickerDock : public QDockWidget {
    Q_OBJECT

public:
    explicit RaidPickerDock(QWidget* parent = nullptr);
    ~RaidPickerDock() override;

signals:
    void raidStarted(const QString& targetUserName);
    void raidCancelled();
    void raidFailed(const QString& error);

public slots:
    void onAuthStateChanged();
    void onPresetChanged(const FilterPreset& preset);
    void onPickRandomClicked();
    void onPickAnotherClicked();
    void onLaunchRaidClicked();
    void onAddToGreylistClicked();
    void onRaidFromFollowedToggled(bool checked);
    void onManageGreylistClicked();
    void onCreatePresetClicked();

private slots:
    void onStreamsReceived(const QList<Streamer>& streamers);
    void onFollowerCountReceived(int followerCount);
    void onRaidResult(bool success, const QString& error);

private:
    void setupUi();
    void setupConnections();
    void updateAuthState();
    void showLoading(const QString& message);
    void showError(const QString& message);
    void showStreamerPreview(const Streamer& streamer);
    void clearPreview();
    void fetchStreams();
    void fetchFollowedStreams();
    void fetchFollowerCountAndShow(const Streamer& streamer);

    // UI Components
    QWidget* m_contentWidget = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
    QStackedWidget* m_stackedWidget = nullptr;

    // Auth UI
    QWidget* m_authPage = nullptr;
    QPushButton* m_loginButton = nullptr;
    QLabel* m_userLabel = nullptr;
    QPushButton* m_logoutButton = nullptr;

    // Main UI
    QWidget* m_mainPage = nullptr;
    PresetSelector* m_presetSelector = nullptr;
    QPushButton* m_createPresetButton = nullptr;
    QPushButton* m_raidFromFollowedButton = nullptr;
    QPushButton* m_pickRandomButton = nullptr;
    StreamerPreview* m_streamerPreview = nullptr;
    QLabel* m_loadingLabel = nullptr;
    QLabel* m_errorLabel = nullptr;
    QPushButton* m_retryButton = nullptr;
    QPushButton* m_manageGreylistButton = nullptr;

    // Services
    StreamerSelector* m_streamerSelector = nullptr;

    // State
    Streamer* m_currentStreamer = nullptr;
    FilterPreset* m_currentPreset = nullptr;
    Streamer m_pendingStreamer; // Streamer waiting for follower count
    bool m_raidFromFollowed = false;
};
