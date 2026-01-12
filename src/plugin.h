#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>

#define HOO_RAID_VERSION "1.0.0"
#define HOO_RAID_PLUGIN_NAME "hoo-raid"
#define HOO_RAID_DISPLAY_NAME "HOO-Raid"

// Forward declarations
class RaidPickerDock;
class OAuthManager;
class TwitchApi;
class SettingsManager;
class CredentialStore;

// Plugin-wide singletons (managed by plugin lifecycle)
namespace HooRaid {
    RaidPickerDock* getDock();
    OAuthManager* getOAuthManager();
    TwitchApi* getTwitchApi();
    SettingsManager* getSettingsManager();
    CredentialStore* getCredentialStore();
}
