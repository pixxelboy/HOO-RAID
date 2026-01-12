/*
 * HOO-RAID Twitch API Credentials Template
 *
 * To configure your development environment:
 *
 * 1. Copy this file to 'secrets.local.h' (in the same directory)
 * 2. Register an application at https://dev.twitch.tv/console/apps
 * 3. Set OAuth Redirect URL to: http://localhost:17563/callback
 * 4. Replace the placeholder values below with your credentials
 *
 * IMPORTANT: Never commit secrets.local.h to version control!
 * It is already listed in .gitignore for your safety.
 */

#pragma once

// Your Twitch Application Client ID
// Found in the Twitch Developer Console under your app settings
#define TWITCH_CLIENT_ID "your_client_id_here"

// Your Twitch Application Client Secret
// Found in the Twitch Developer Console under your app settings
// Keep this secret! Never share it publicly.
#define TWITCH_CLIENT_SECRET "your_client_secret_here"
