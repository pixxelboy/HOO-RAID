#include "raid-picker-dock.h"
#include "preset-selector.h"
#include "streamer-preview.h"
#include "filter-editor.h"
#include "greylist-manager.h"
#include "../plugin.h"
#include "../services/oauth-manager.h"
#include "../services/twitch-api.h"
#include "../services/streamer-selector.h"
#include "../storage/settings-manager.h"
#include "../models/streamer.h"
#include "../models/filter-preset.h"
#include "../models/greylist-entry.h"
#include <obs-module.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

RaidPickerDock::RaidPickerDock(QWidget* parent)
    : QDockWidget(parent)
{
    m_streamerSelector = new StreamerSelector(HooRaid::getSettingsManager(), this);

    setupUi();
    setupConnections();
    updateAuthState();
}

RaidPickerDock::~RaidPickerDock()
{
    delete m_currentStreamer;
    delete m_currentPreset;
}

void RaidPickerDock::setupUi()
{
    m_contentWidget = new QWidget(this);
    setWidget(m_contentWidget);

    m_mainLayout = new QVBoxLayout(m_contentWidget);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(8);

    m_stackedWidget = new QStackedWidget(m_contentWidget);
    m_mainLayout->addWidget(m_stackedWidget);

    // Auth page
    m_authPage = new QWidget();
    QVBoxLayout* authLayout = new QVBoxLayout(m_authPage);
    authLayout->setAlignment(Qt::AlignCenter);

    m_loginButton = new QPushButton(obs_module_text("Auth.LoginWithTwitch"), m_authPage);
    m_loginButton->setMinimumHeight(40);
    authLayout->addWidget(m_loginButton);

    m_stackedWidget->addWidget(m_authPage);

    // Main page
    m_mainPage = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_mainPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    // User info bar
    QHBoxLayout* userBar = new QHBoxLayout();
    m_userLabel = new QLabel(m_mainPage);
    userBar->addWidget(m_userLabel);
    userBar->addStretch();
    m_logoutButton = new QPushButton(obs_module_text("Auth.Logout"), m_mainPage);
    m_logoutButton->setMaximumWidth(80);
    userBar->addWidget(m_logoutButton);
    mainLayout->addLayout(userBar);

    // Preset selector row
    QHBoxLayout* presetRow = new QHBoxLayout();
    m_presetSelector = new PresetSelector(m_mainPage);
    presetRow->addWidget(m_presetSelector, 1);

    m_createPresetButton = new QPushButton("+", m_mainPage);
    m_createPresetButton->setMaximumWidth(30);
    m_createPresetButton->setToolTip(obs_module_text("Preset.SaveAsPreset"));
    presetRow->addWidget(m_createPresetButton);
    mainLayout->addLayout(presetRow);

    // Raid from Followed toggle
    m_raidFromFollowedButton = new QPushButton(obs_module_text("Action.RaidFromFollowed"), m_mainPage);
    m_raidFromFollowedButton->setCheckable(true);
    mainLayout->addWidget(m_raidFromFollowedButton);

    // Pick Random button
    m_pickRandomButton = new QPushButton(obs_module_text("Action.PickRandom"), m_mainPage);
    m_pickRandomButton->setMinimumHeight(36);
    mainLayout->addWidget(m_pickRandomButton);

    // Loading indicator
    m_loadingLabel = new QLabel(obs_module_text("Status.Loading"), m_mainPage);
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->hide();
    mainLayout->addWidget(m_loadingLabel);

    // Error display
    m_errorLabel = new QLabel(m_mainPage);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // Retry button
    m_retryButton = new QPushButton(obs_module_text("Action.Retry"), m_mainPage);
    m_retryButton->hide();
    mainLayout->addWidget(m_retryButton);

    // Streamer preview
    m_streamerPreview = new StreamerPreview(m_mainPage);
    m_streamerPreview->hide();
    mainLayout->addWidget(m_streamerPreview);

    mainLayout->addStretch();

    // Manage Greylist button at bottom
    m_manageGreylistButton = new QPushButton(obs_module_text("Action.ManageGreylist"), m_mainPage);
    mainLayout->addWidget(m_manageGreylistButton);

    m_stackedWidget->addWidget(m_mainPage);
}

void RaidPickerDock::setupConnections()
{
    // Auth connections
    connect(m_loginButton, &QPushButton::clicked, this, []() {
        HooRaid::getOAuthManager()->startAuthFlow();
    });

    connect(m_logoutButton, &QPushButton::clicked, this, []() {
        HooRaid::getOAuthManager()->logout();
    });

    OAuthManager* oauth = HooRaid::getOAuthManager();
    if (oauth) {
        connect(oauth, &OAuthManager::authenticationSuccess,
                this, &RaidPickerDock::onAuthStateChanged);
        connect(oauth, &OAuthManager::loggedOut,
                this, &RaidPickerDock::onAuthStateChanged);
        connect(oauth, &OAuthManager::authenticationFailed,
                this, [this](const QString& error) {
                    showError(error);
                    onAuthStateChanged();
                });
    }

    // Twitch API connections
    TwitchApi* api = HooRaid::getTwitchApi();
    if (api) {
        connect(api, &TwitchApi::rateLimited, this, [this](int seconds) {
            showError(QString(obs_module_text("Error.RateLimited")).arg(seconds));
        });
        connect(api, &TwitchApi::unauthorized, this, [this]() {
            showError(obs_module_text("Auth.TokenExpired"));
            HooRaid::getOAuthManager()->refreshToken();
        });
    }

    // Preset connections
    connect(m_presetSelector, &PresetSelector::presetChanged,
            this, &RaidPickerDock::onPresetChanged);
    connect(m_presetSelector, &PresetSelector::editPresetClicked,
            this, [this](const FilterPreset& preset) {
                FilterEditor* editor = new FilterEditor(preset, this);
                connect(editor, &FilterEditor::presetSaved, m_presetSelector, &PresetSelector::updateCustomPreset);
                editor->exec();
                editor->deleteLater();
            });

    // Create preset button
    connect(m_createPresetButton, &QPushButton::clicked,
            this, &RaidPickerDock::onCreatePresetClicked);

    // Raid from followed toggle
    connect(m_raidFromFollowedButton, &QPushButton::toggled,
            this, &RaidPickerDock::onRaidFromFollowedToggled);

    // Action buttons
    connect(m_pickRandomButton, &QPushButton::clicked,
            this, &RaidPickerDock::onPickRandomClicked);
    connect(m_retryButton, &QPushButton::clicked,
            this, &RaidPickerDock::onPickRandomClicked);

    // Streamer preview connections
    connect(m_streamerPreview, &StreamerPreview::pickAnotherClicked,
            this, &RaidPickerDock::onPickAnotherClicked);
    connect(m_streamerPreview, &StreamerPreview::launchRaidClicked,
            this, &RaidPickerDock::onLaunchRaidClicked);
    connect(m_streamerPreview, &StreamerPreview::addToGreylistClicked,
            this, &RaidPickerDock::onAddToGreylistClicked);

    // Manage greylist button
    connect(m_manageGreylistButton, &QPushButton::clicked,
            this, &RaidPickerDock::onManageGreylistClicked);

    // Streamer selector connections
    connect(m_streamerSelector, &StreamerSelector::poolExhausted, this, [this]() {
        if (m_raidFromFollowed) {
            showError(obs_module_text("Error.NoFollowedLive"));
        } else {
            showError(obs_module_text("Error.NoMatches"));
        }
    });
}

void RaidPickerDock::updateAuthState()
{
    OAuthManager* oauth = HooRaid::getOAuthManager();
    if (!oauth) return;

    if (oauth->isAuthenticated()) {
        const OAuthCredentials& creds = oauth->credentials();
        m_userLabel->setText(QString(obs_module_text("Auth.LoggedInAs")).arg(creds.userName));
        m_stackedWidget->setCurrentWidget(m_mainPage);
    } else {
        m_stackedWidget->setCurrentWidget(m_authPage);
    }
}

void RaidPickerDock::onAuthStateChanged()
{
    updateAuthState();
    m_streamerSelector->reset();
    clearPreview();
}

void RaidPickerDock::onPresetChanged(const FilterPreset& preset)
{
    delete m_currentPreset;
    m_currentPreset = new FilterPreset(preset);

    // Update the selector filter
    m_streamerSelector->setFilter(preset);
    clearPreview();

    // Save last used preset
    SettingsManager* settings = HooRaid::getSettingsManager();
    if (settings) {
        settings->setLastPresetId(preset.id);
    }
}

void RaidPickerDock::onPickRandomClicked()
{
    if (!m_currentPreset) {
        // Use first preset if none selected
        m_currentPreset = new FilterPreset(m_presetSelector->currentPreset());
    }

    // If we have streamers in the pool, try to select from it
    if (m_streamerSelector->poolSize() > 0) {
        Streamer selected = m_streamerSelector->selectRandom();
        if (selected.isValid()) {
            fetchFollowerCountAndShow(selected);
            return;
        }
    }

    // Otherwise fetch new streams
    if (m_raidFromFollowed) {
        fetchFollowedStreams();
    } else {
        fetchStreams();
    }
}

void RaidPickerDock::onPickAnotherClicked()
{
    // Try to select next from existing pool
    if (m_streamerSelector->hasMore()) {
        Streamer selected = m_streamerSelector->selectNext();
        if (selected.isValid()) {
            fetchFollowerCountAndShow(selected);
            return;
        }
    }

    // Pool exhausted, fetch new streams
    if (m_raidFromFollowed) {
        fetchFollowedStreams();
    } else {
        fetchStreams();
    }
}

void RaidPickerDock::onLaunchRaidClicked()
{
    if (!m_currentStreamer) return;

    OAuthManager* oauth = HooRaid::getOAuthManager();
    if (!oauth || !oauth->isAuthenticated()) {
        showError(obs_module_text("Auth.AuthenticationFailed"));
        return;
    }

    showLoading(obs_module_text("Status.Loading"));

    TwitchApi* api = HooRaid::getTwitchApi();
    if (!api) {
        showError(obs_module_text("Error.ApiError"));
        return;
    }

    QString fromId = oauth->credentials().userId;
    QString toId = m_currentStreamer->userId;
    QString targetName = m_currentStreamer->userName;

    api->startRaid(fromId, toId, [this, targetName](const ApiResponse<bool>& response) {
        if (response.isSuccess()) {
            onRaidResult(true, QString());
            emit raidStarted(targetName);
        } else {
            onRaidResult(false, response.errorMessage);
            emit raidFailed(response.errorMessage);
        }
    });
}

void RaidPickerDock::onAddToGreylistClicked()
{
    if (!m_currentStreamer) return;

    SettingsManager* settings = HooRaid::getSettingsManager();
    if (settings) {
        GreylistEntry entry = GreylistEntry::fromStreamer(
            m_currentStreamer->userId,
            m_currentStreamer->userLogin,
            m_currentStreamer->userName
        );
        settings->addToGreylist(entry);

        // Remove from selector pool
        m_streamerSelector->excludeFromPool(m_currentStreamer->userId);

        blog(LOG_INFO, "[HOO-Raid] Added to greylist: %s",
             m_currentStreamer->userName.toUtf8().constData());
    }

    // Auto-advance to next streamer
    onPickAnotherClicked();
}

void RaidPickerDock::onRaidFromFollowedToggled(bool checked)
{
    m_raidFromFollowed = checked;
    m_streamerSelector->reset();
    clearPreview();

    // Update button text
    m_pickRandomButton->setText(checked
                                    ? obs_module_text("Action.PickRandom")
                                    : obs_module_text("Action.PickRandom"));
}

void RaidPickerDock::onManageGreylistClicked()
{
    GreylistManager* manager = new GreylistManager(this);
    manager->exec();
    manager->deleteLater();
}

void RaidPickerDock::onCreatePresetClicked()
{
    FilterEditor* editor = new FilterEditor(this);

    // Pre-fill with current preset if custom
    if (m_currentPreset && !m_currentPreset->isBuiltIn) {
        editor->setPreset(*m_currentPreset);
    }

    connect(editor, &FilterEditor::presetSaved, this, [this](const FilterPreset& preset) {
        m_presetSelector->addCustomPreset(preset);
        m_presetSelector->setCurrentPreset(preset.id);
    });

    editor->exec();
    editor->deleteLater();
}

void RaidPickerDock::fetchStreams()
{
    if (!m_currentPreset) {
        showError(obs_module_text("Error.NoMatches"));
        return;
    }

    showLoading(obs_module_text("Status.Searching"));

    TwitchApi* api = HooRaid::getTwitchApi();
    if (!api) {
        showError(obs_module_text("Error.ApiError"));
        return;
    }

    api->fetchStreams(
        m_currentPreset->language,
        m_currentPreset->gameId,
        100, // Fetch maximum streams
        [this](const ApiResponse<QList<Streamer>>& response) {
            if (response.isSuccess()) {
                onStreamsReceived(response.data);
            } else {
                showError(response.errorMessage.isEmpty()
                          ? obs_module_text("Error.ApiError")
                          : QString(obs_module_text("Error.ApiError")).arg(response.errorMessage));
            }
        });
}

void RaidPickerDock::fetchFollowedStreams()
{
    OAuthManager* oauth = HooRaid::getOAuthManager();
    if (!oauth || !oauth->isAuthenticated()) {
        showError(obs_module_text("Auth.AuthenticationFailed"));
        return;
    }

    showLoading(obs_module_text("Status.Searching"));

    TwitchApi* api = HooRaid::getTwitchApi();
    if (!api) {
        showError(obs_module_text("Error.ApiError"));
        return;
    }

    QString userId = oauth->credentials().userId;

    api->fetchFollowedStreams(userId, 100,
        [this](const ApiResponse<QList<Streamer>>& response) {
            if (response.isSuccess()) {
                if (response.data.isEmpty()) {
                    showError(obs_module_text("Error.NoFollowedLive"));
                } else {
                    onStreamsReceived(response.data);
                }
            } else {
                showError(response.errorMessage.isEmpty()
                          ? obs_module_text("Error.ApiError")
                          : QString(obs_module_text("Error.ApiError")).arg(response.errorMessage));
            }
        });
}

void RaidPickerDock::onStreamsReceived(const QList<Streamer>& streamers)
{
    if (streamers.isEmpty()) {
        if (m_raidFromFollowed) {
            showError(obs_module_text("Error.NoFollowedLive"));
        } else {
            showError(obs_module_text("Error.NoMatches"));
        }
        return;
    }

    // Update the pool and apply filter
    m_streamerSelector->setPool(streamers);

    if (m_streamerSelector->poolSize() == 0) {
        // All streamers filtered out
        if (m_raidFromFollowed) {
            showError(obs_module_text("Error.NoFollowedLive"));
        } else {
            showError(QString("%1\n%2")
                          .arg(obs_module_text("Error.NoMatches"))
                          .arg(obs_module_text("Error.NoMatchesSuggestion")));
        }
        return;
    }

    // Select a random streamer
    Streamer selected = m_streamerSelector->selectRandom();
    if (selected.isValid()) {
        fetchFollowerCountAndShow(selected);
    } else {
        showError(obs_module_text("Error.NoMatches"));
    }
}

void RaidPickerDock::fetchFollowerCountAndShow(const Streamer& streamer)
{
    m_pendingStreamer = streamer;
    showLoading(obs_module_text("Status.FetchingFollowers"));

    TwitchApi* api = HooRaid::getTwitchApi();
    if (!api) {
        // Show without follower count
        showStreamerPreview(streamer);
        return;
    }

    api->fetchFollowerCount(streamer.userId, [this](const ApiResponse<int>& response) {
        if (response.isSuccess()) {
            onFollowerCountReceived(response.data);
        } else {
            // Show streamer even if follower count fails
            showStreamerPreview(m_pendingStreamer);
        }
    });
}

void RaidPickerDock::onFollowerCountReceived(int followerCount)
{
    m_pendingStreamer.followerCount = followerCount;

    // Check if streamer still matches filter after getting follower count
    if (m_currentPreset && !m_currentPreset->matchesFollowerCount(followerCount)) {
        blog(LOG_DEBUG, "[HOO-Raid] Streamer %s rejected: follower count %d out of range",
             m_pendingStreamer.userName.toUtf8().constData(), followerCount);

        // Try next streamer
        m_streamerSelector->excludeFromPool(m_pendingStreamer.userId);
        onPickAnotherClicked();
        return;
    }

    showStreamerPreview(m_pendingStreamer);
}

void RaidPickerDock::onRaidResult(bool success, const QString& error)
{
    m_loadingLabel->hide();

    if (success) {
        // Show success feedback
        m_errorLabel->setStyleSheet("color: green;");
        m_errorLabel->setText(obs_module_text("Raid.Queued"));
        m_errorLabel->show();
        m_retryButton->hide();

        blog(LOG_INFO, "[HOO-Raid] Raid queued successfully");
    } else {
        m_errorLabel->setStyleSheet("color: red;");
        m_errorLabel->setText(QString(obs_module_text("Raid.Failed")).arg(error));
        m_errorLabel->show();
        m_retryButton->hide();

        blog(LOG_WARNING, "[HOO-Raid] Raid failed: %s", error.toUtf8().constData());
    }

    m_pickRandomButton->setEnabled(true);
}

void RaidPickerDock::showLoading(const QString& message)
{
    m_loadingLabel->setText(message);
    m_loadingLabel->show();
    m_errorLabel->hide();
    m_retryButton->hide();
    m_streamerPreview->hide();
    m_pickRandomButton->setEnabled(false);
}

void RaidPickerDock::showError(const QString& message)
{
    m_loadingLabel->hide();
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setText(message);
    m_errorLabel->show();
    m_retryButton->show();
    m_streamerPreview->hide();
    m_pickRandomButton->setEnabled(true);
}

void RaidPickerDock::showStreamerPreview(const Streamer& streamer)
{
    delete m_currentStreamer;
    m_currentStreamer = new Streamer(streamer);

    m_loadingLabel->hide();
    m_errorLabel->hide();
    m_retryButton->hide();
    m_streamerPreview->setStreamer(streamer);
    m_streamerPreview->show();
    m_pickRandomButton->setEnabled(true);
}

void RaidPickerDock::clearPreview()
{
    delete m_currentStreamer;
    m_currentStreamer = nullptr;

    m_loadingLabel->hide();
    m_errorLabel->hide();
    m_retryButton->hide();
    m_streamerPreview->hide();
}
