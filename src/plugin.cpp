#include "plugin.h"
#include "ui/raid-picker-dock.h"
#include "services/oauth-manager.h"
#include "services/twitch-api.h"
#include "storage/settings-manager.h"
#include "storage/credential-store.h"

#include <QMainWindow>
#include <QAction>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(HOO_RAID_PLUGIN_NAME, "en-US")

// Plugin-wide instances
static RaidPickerDock* s_dock = nullptr;
static OAuthManager* s_oauthManager = nullptr;
static TwitchApi* s_twitchApi = nullptr;
static SettingsManager* s_settingsManager = nullptr;
static CredentialStore* s_credentialStore = nullptr;

namespace HooRaid {
    RaidPickerDock* getDock() { return s_dock; }
    OAuthManager* getOAuthManager() { return s_oauthManager; }
    TwitchApi* getTwitchApi() { return s_twitchApi; }
    SettingsManager* getSettingsManager() { return s_settingsManager; }
    CredentialStore* getCredentialStore() { return s_credentialStore; }
}

static void initializeServices()
{
    // Initialize in dependency order
    s_credentialStore = new CredentialStore();
    s_settingsManager = new SettingsManager();
    s_oauthManager = new OAuthManager(s_credentialStore);
    s_twitchApi = new TwitchApi(s_oauthManager);

    // Load persisted settings
    s_settingsManager->load();

    // Attempt to restore OAuth session from stored credentials
    s_oauthManager->restoreSession();
}

static void shutdownServices()
{
    // Save settings before shutdown
    if (s_settingsManager) {
        s_settingsManager->save();
    }

    // Cleanup in reverse order
    delete s_twitchApi;
    s_twitchApi = nullptr;

    delete s_oauthManager;
    s_oauthManager = nullptr;

    delete s_settingsManager;
    s_settingsManager = nullptr;

    delete s_credentialStore;
    s_credentialStore = nullptr;
}

static void registerDock()
{
    QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainWindow) {
        blog(LOG_ERROR, "[HOO-Raid] Failed to get OBS main window");
        return;
    }

    s_dock = new RaidPickerDock(mainWindow);
    s_dock->setObjectName("HooRaidDock");
    s_dock->setWindowTitle(obs_module_text("RaidPicker"));
    s_dock->setFeatures(QDockWidget::DockWidgetClosable |
                        QDockWidget::DockWidgetMovable |
                        QDockWidget::DockWidgetFloatable);

    mainWindow->addDockWidget(Qt::RightDockWidgetArea, s_dock);

    // Add menu action to show/hide the dock
    QAction* action = static_cast<QAction*>(obs_frontend_add_dock(s_dock));
    if (action) {
        action->setText(obs_module_text("RaidPicker"));
    }

    blog(LOG_INFO, "[HOO-Raid] Dock widget registered successfully");
}

static void unregisterDock()
{
    if (s_dock) {
        // Dock is parented to main window, will be deleted with it
        // but we should remove it from the dock management
        s_dock->setParent(nullptr);
        delete s_dock;
        s_dock = nullptr;
    }
}

bool obs_module_load(void)
{
    blog(LOG_INFO, "[HOO-Raid] Plugin version %s loading...", HOO_RAID_VERSION);

    // Initialize services first
    initializeServices();

    // Register the dock widget with OBS
    registerDock();

    blog(LOG_INFO, "[HOO-Raid] Plugin loaded successfully");
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "[HOO-Raid] Plugin unloading...");

    // Unregister dock first
    unregisterDock();

    // Shutdown services
    shutdownServices();

    blog(LOG_INFO, "[HOO-Raid] Plugin unloaded");
}

const char* obs_module_name(void)
{
    return HOO_RAID_DISPLAY_NAME;
}

const char* obs_module_description(void)
{
    return obs_module_text("RaidPickerDescription");
}
