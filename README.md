# HOO-RAID

### *Because who to raid sometimes needs a fast answer*

---

<p align="center">
  <img src="docs/images/hoo-raid-banner.png" alt="HOO-RAID Banner" width="600">
</p>

<p align="center">
  <strong>The smart raid picker plugin for OBS Studio</strong><br>
  Find and raid smaller streamers with intelligent filters in seconds
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#configuration">Configuration</a> •
  <a href="#usage">Usage</a> •
  <a href="#building-from-source">Build</a> •
  <a href="#contributing">Contributing</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/OBS%20Studio-30.0%2B-blue" alt="OBS Version">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey" alt="Platforms">
  <img src="https://img.shields.io/badge/License-MIT-green" alt="License">
  <img src="https://img.shields.io/badge/Version-1.0.0-orange" alt="Version">
</p>

---

## The Problem

You've just finished an amazing stream. Your community is hyped. It's time to raid... but **who**?

- Scrolling through Twitch takes forever
- You want to support smaller creators, not just big names
- Finding someone in your language/category is a hassle
- You keep accidentally raiding the same people

**HOO-RAID solves all of this.**

---

## Features

### Instant Streamer Discovery

Pick a random streamer matching your criteria with a single click. No more endless scrolling.

### Smart Filter Presets

Three built-in presets to find streamers at different growth stages:

| Preset | Followers | Viewers | Best For |
|--------|-----------|---------|----------|
| **Beginner** | 0-100 | 1-5 | Supporting brand new streamers |
| **Small** | 100-1K | 5-25 | Growing creators who need a boost |
| **Medium** | 1K-10K | 25-100 | Established small streamers |

### Custom Filters

Create your own filter presets with:
- **Language** - Raid streamers your community can understand
- **Category/Game** - Stay within your niche or explore others
- **Viewer count** - Target specific audience sizes
- **Follower count** - Support creators at any growth stage

### Raid from Followed

Already follow great small streamers? Toggle "Raid from Followed" to pick randomly from your live follows.

### Greylist Management

- Add streamers to your greylist with one click
- They won't appear in future suggestions
- Manage your greylist anytime
- Perfect for avoiding streamers you've already raided recently

### Streamer Preview

Before you raid, see:
- Stream title and category
- Current viewer count
- Follower count
- Stream duration
- Live thumbnail

### Seamless Integration

- Native OBS dock widget
- Works on Windows, macOS, and Linux
- Secure OAuth authentication
- Settings persist across restarts
- Supports English and French (more coming!)

---

## Screenshots

<p align="center">
  <img src="docs/images/screenshot-main.png" alt="Main Interface" width="300">
  <img src="docs/images/screenshot-preview.png" alt="Streamer Preview" width="300">
</p>

<p align="center">
  <img src="docs/images/screenshot-filters.png" alt="Custom Filters" width="300">
  <img src="docs/images/screenshot-greylist.png" alt="Greylist Manager" width="300">
</p>

---

## Installation

### Requirements

- **OBS Studio** 30.0 or later
- **Twitch Account** (for authentication)
- **Operating System**: Windows 10+, macOS 10.15+, or Linux (Ubuntu 20.04+)

### Download

Download the latest release for your platform:

| Platform | Download |
|----------|----------|
| Windows (64-bit) | [hoo-raid-windows-x64.zip](https://github.com/YOUR_USERNAME/HOO-Raid/releases/latest) |
| macOS (Intel/Apple Silicon) | [hoo-raid-macos.zip](https://github.com/YOUR_USERNAME/HOO-Raid/releases/latest) |
| Linux (64-bit) | [hoo-raid-linux-x64.tar.gz](https://github.com/YOUR_USERNAME/HOO-Raid/releases/latest) |

### Install Steps

#### Windows

1. Download `hoo-raid-windows-x64.zip`
2. Extract the archive
3. Copy `hoo-raid.dll` to:
   ```
   %APPDATA%\obs-studio\plugins\hoo-raid\bin\64bit\
   ```
4. Copy the `data` folder to:
   ```
   %APPDATA%\obs-studio\plugins\hoo-raid\data\
   ```
5. Restart OBS Studio
6. Go to **View → Docks → HOO-Raid**

#### macOS

1. Download `hoo-raid-macos.zip`
2. Extract the archive
3. Copy `hoo-raid.so` to:
   ```
   ~/Library/Application Support/obs-studio/plugins/hoo-raid/bin/
   ```
4. Copy the `data` folder to:
   ```
   ~/Library/Application Support/obs-studio/plugins/hoo-raid/data/
   ```
5. Restart OBS Studio
6. Go to **View → Docks → HOO-Raid**

#### Linux

1. Download `hoo-raid-linux-x64.tar.gz`
2. Extract the archive:
   ```bash
   tar -xzf hoo-raid-linux-x64.tar.gz
   ```
3. Copy `hoo-raid.so` to:
   ```bash
   mkdir -p ~/.config/obs-studio/plugins/hoo-raid/bin/64bit/
   cp hoo-raid.so ~/.config/obs-studio/plugins/hoo-raid/bin/64bit/
   ```
4. Copy the `data` folder:
   ```bash
   cp -r data ~/.config/obs-studio/plugins/hoo-raid/data/
   ```
5. Install required dependency:
   ```bash
   # Ubuntu/Debian
   sudo apt install libsecret-1-0

   # Fedora
   sudo dnf install libsecret

   # Arch
   sudo pacman -S libsecret
   ```
6. Restart OBS Studio
7. Go to **View → Docks → HOO-Raid**

### Verify Installation

1. Open OBS Studio
2. Go to **Help → Log Files → View Current Log**
3. Search for `HOO-Raid` - you should see:
   ```
   [HOO-Raid] Plugin version 1.0.0 loading...
   [HOO-Raid] Plugin loaded successfully
   ```

---

## Configuration

### First-Time Setup

1. **Open the HOO-Raid dock** (View → Docks → HOO-Raid)
2. **Click "Login with Twitch"**
3. **Authorize the application** in your browser
4. Once authenticated, you'll see your Twitch username in the dock

### Required Twitch Permissions

HOO-RAID requests the following permissions:

| Permission | Purpose |
|------------|---------|
| `user:read:follows` | Access your followed streamers list |
| `channel:manage:raids` | Execute raids from your channel |

These are the minimum permissions required. HOO-RAID does not access your chat, subscriptions, or any other data.

### Settings Persistence

All your settings are automatically saved:
- Custom filter presets
- Greylist entries
- Last used preset
- Authentication tokens (stored securely in your system keychain)

Settings are stored in:
- **Windows**: `%APPDATA%\obs-studio\plugin_config\hoo-raid\`
- **macOS**: `~/Library/Application Support/obs-studio/plugin_config/hoo-raid/`
- **Linux**: `~/.config/obs-studio/plugin_config/hoo-raid/`

---

## Usage

### Quick Start

1. **Select a preset** from the dropdown (Beginner, Small, or Medium)
2. **Click "Pick Random"** to find a streamer
3. **Review the preview** - check their stream title, category, and stats
4. **Click "Launch Raid"** to start the raid, or
5. **Click "Pick Another"** to see a different streamer

### Using Custom Filters

1. Click the **"+"** button next to the preset dropdown
2. Configure your filter:
   - **Name**: Give your preset a memorable name
   - **Language**: Select a language or leave as "Any"
   - **Category**: Start typing to search for a game/category
   - **Viewers**: Set min/max viewer count (0 = no limit)
   - **Followers**: Set min/max follower count (0 = no limit)
3. Click **Save**
4. Your preset appears in the dropdown

### Raiding from Followed Streamers

1. Toggle **"Raid from Followed"** button
2. Click **"Pick Random"**
3. HOO-RAID picks from your followed streamers who are currently live
4. Filters still apply - combine with presets for targeted discovery

### Managing the Greylist

**Adding to Greylist:**
- When previewing a streamer, click **"Add to Greylist"**
- They won't appear in future picks
- HOO-RAID automatically shows the next streamer

**Viewing/Removing:**
1. Click **"Manage Greylist"** at the bottom of the dock
2. See all greylisted streamers with dates added
3. Select and click **"Remove"** to un-greylist
4. Click **"Clear All"** to reset the entire list

### Tips for Effective Raids

1. **Raid regularly** - Small streamers remember and appreciate it
2. **Match your community** - Use language filters for better engagement
3. **Vary your targets** - Use the greylist to avoid repeating raids
4. **Check the preview** - Make sure the stream content is appropriate
5. **Announce the raid** - Tell your chat who you're raiding and why

---

## Building from Source

### Prerequisites

| Tool | Version | Purpose |
|------|---------|---------|
| CMake | 3.16+ | Build system |
| Qt | 6.2+ | UI framework |
| C++ Compiler | C++17 | GCC 9+, Clang 10+, MSVC 2019+ |
| OBS Studio | 30.0+ | Plugin SDK |
| Git | Any | Version control |

**Platform-Specific:**

- **Windows**: Visual Studio 2019/2022 with C++ workload
- **macOS**: Xcode 13+
- **Linux**: `libsecret-1-dev`, `qtbase6-dev`, `libobs-dev`

### Clone and Build

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/HOO-Raid.git
cd HOO-Raid

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure (adjust paths as needed)
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Build
cmake --build . --config RelWithDebInfo
```

### Configure Twitch Credentials

1. Go to [Twitch Developer Console](https://dev.twitch.tv/console/apps)
2. Create a new application:
   - **Name**: `HOO-Raid Development`
   - **OAuth Redirect URL**: `http://localhost:17563/callback`
   - **Category**: `Application Integration`
3. Copy your Client ID and Client Secret
4. Update the credentials in:
   - `src/services/oauth-manager.h` (line 47)
   - `src/services/twitch-api.h` (line 97)

### Install Locally

```bash
# Windows (PowerShell)
copy build\RelWithDebInfo\hoo-raid.dll "$env:APPDATA\obs-studio\plugins\hoo-raid\bin\64bit\"

# macOS
cp build/hoo-raid.so ~/Library/Application\ Support/obs-studio/plugins/hoo-raid/bin/

# Linux
cp build/hoo-raid.so ~/.config/obs-studio/plugins/hoo-raid/bin/64bit/
```

### Verify Build

```bash
# Check plugin loads (look for HOO-Raid in log)
# OBS: Help → Log Files → View Current Log

# Expected output:
# [HOO-Raid] Plugin version 1.0.0 loading...
# [HOO-Raid] Dock widget registered successfully
# [HOO-Raid] Plugin loaded successfully
```

---

## Troubleshooting

### Plugin Not Showing in OBS

1. **Check installation path** - Ensure files are in the correct `plugins` folder
2. **Check OBS version** - Requires OBS 30.0 or later
3. **Check the log** - Help → Log Files → View Current Log
4. **Restart OBS** - Plugins load on startup only

### "Login with Twitch" Not Working

1. **Check your browser** - The auth page should open automatically
2. **Check firewall** - Port 17563 must be available on localhost
3. **Try again** - Sometimes Twitch is slow to respond
4. **Check Twitch status** - [status.twitch.tv](https://status.twitch.tv)

### "No Matches Found" Error

This means no streamers match your current filters. Try:
- Widening your viewer/follower ranges
- Removing language or category filters
- Using a different preset
- Checking if your followed streamers are live (for "Raid from Followed")

### Rate Limit Errors

Twitch limits API requests. If you see rate limit errors:
- Wait for the countdown to finish
- Avoid clicking "Pick Random" repeatedly in quick succession
- The plugin automatically respects rate limits

### Authentication Expired

If you see "Session expired":
1. The plugin will attempt to refresh automatically
2. If that fails, click "Login with Twitch" again
3. Your presets and greylist are preserved

### Settings Not Saving

1. **Check permissions** - Ensure OBS can write to its config folder
2. **Don't force-quit OBS** - Settings save on normal shutdown
3. **Check disk space** - Ensure you have free disk space

---

## FAQ

**Q: Is this safe to use?**
A: Yes! HOO-RAID only requests the minimum permissions needed, stores credentials securely in your system keychain, and is open source for full transparency.

**Q: Does this violate Twitch TOS?**
A: No. HOO-RAID uses official Twitch APIs exactly as intended. Raiding is a supported Twitch feature.

**Q: Can I get banned for using this?**
A: No. This is a legitimate tool that helps you discover and support smaller streamers.

**Q: How is this different from Twitch's built-in raid suggestions?**
A: Twitch shows suggestions but doesn't let you filter by follower count, save presets, or maintain a greylist. HOO-RAID gives you full control.

**Q: Can I contribute translations?**
A: Yes! See the Contributing section below.

**Q: Will you add feature X?**
A: Open an issue on GitHub! We welcome feature requests.

---

## Contributing

We welcome contributions! Here's how to help:

### Reporting Bugs

1. Check [existing issues](https://github.com/YOUR_USERNAME/HOO-Raid/issues) first
2. Open a new issue with:
   - OS and OBS version
   - Steps to reproduce
   - Expected vs actual behavior
   - OBS log file (Help → Log Files → Upload Current Log)

### Suggesting Features

1. Open an issue with the `enhancement` label
2. Describe the feature and why it's useful
3. Include mockups if applicable

### Adding Translations

1. Copy `src/i18n/en-US.json` to a new file (e.g., `de-DE.json`)
2. Translate all strings
3. Submit a pull request

### Code Contributions

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Test thoroughly in OBS
5. Submit a pull request

### Code Style

- C++17 standard
- Qt coding conventions
- Use `blog()` for logging (OBS log system)
- Prefix classes with their module (e.g., `RaidPickerDock`)

---

## Roadmap

### Version 1.1 (Planned)
- [ ] Favorite streamers list (priority raid targets)
- [ ] Raid history with stats
- [ ] Keyboard shortcuts
- [ ] More language translations

### Version 1.2 (Planned)
- [ ] Raid scheduling (auto-raid after stream ends)
- [ ] Community raid pools (share presets)
- [ ] Integration with stream alerts

### Future Ideas
- Team raid coordination
- Raid analytics dashboard
- Streamer notes/tags

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- [OBS Project](https://obsproject.com/) - For the amazing streaming software
- [Twitch](https://dev.twitch.tv/) - For the comprehensive API
- [Qt Project](https://www.qt.io/) - For the UI framework
- [QtKeychain](https://github.com/frankosterfeld/qtkeychain) - For secure credential storage
- All the small streamers who inspire us to build tools that support them

---

## Support the Project

If HOO-RAID helps you discover amazing streamers:

- **Star this repository** - It helps others find us
- **Share with fellow streamers** - Spread the word
- **Report bugs and suggest features** - Help us improve
- **Contribute code or translations** - Join the development

---

<p align="center">
  <strong>Happy Raiding!</strong><br>
  <em>Made with love for the streaming community</em>
</p>

<p align="center">
  <a href="https://github.com/YOUR_USERNAME/HOO-Raid/issues">Report Bug</a> •
  <a href="https://github.com/YOUR_USERNAME/HOO-Raid/issues">Request Feature</a> •
  <a href="https://github.com/YOUR_USERNAME/HOO-Raid/discussions">Discussions</a>
</p>
