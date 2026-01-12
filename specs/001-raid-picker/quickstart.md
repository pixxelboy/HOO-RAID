# Quickstart: HOO-Raid Plugin Development

**Feature**: 001-raid-picker
**Date**: 2026-01-11

## Prerequisites

### Required Software

| Tool | Version | Purpose |
|------|---------|---------|
| OBS Studio | 30.0+ | Runtime environment |
| CMake | 3.16+ | Build system |
| Qt | 6.2+ | UI framework (must match OBS build) |
| C++ Compiler | C++17 support | GCC 9+, Clang 10+, MSVC 2019+ |
| Git | Any | Version control |

### Platform-Specific

**Windows**:
- Visual Studio 2019 or 2022 (with C++ workload)
- Windows SDK 10.0.20348.0+

**macOS**:
- Xcode 13+
- macOS SDK 10.15+

**Linux**:
- libsecret-1-dev (for credential storage)
- OBS development packages

---

## Setup

### 1. Clone Repository

```bash
git clone https://github.com/YOUR_USERNAME/HOO-Raid.git
cd HOO-Raid
```

### 2. Install OBS Plugin Template Dependencies

```bash
# The OBS plugin template is included as a submodule
git submodule update --init --recursive
```

### 3. Configure Twitch Application

1. Go to [Twitch Developer Console](https://dev.twitch.tv/console/apps)
2. Create a new application:
   - Name: `HOO-Raid (Development)`
   - OAuth Redirect URL: `http://localhost:3000/callback`
   - Category: `Application Integration`
3. Note your **Client ID** and **Client Secret**
4. Create `config/secrets.local.h` (gitignored):
   ```cpp
   #pragma once
   #define TWITCH_CLIENT_ID "your_client_id_here"
   #define TWITCH_CLIENT_SECRET "your_client_secret_here"
   ```

### 4. Build the Plugin

**Windows (Visual Studio)**:
```powershell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config RelWithDebInfo
```

**macOS**:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

**Linux**:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

### 5. Install for Testing

The build outputs the plugin to `build/` directory. To test:

**Windows**:
```powershell
# Copy to OBS plugins folder
copy build\RelWithDebInfo\hoo-raid.dll "%APPDATA%\obs-studio\plugins\hoo-raid\bin\64bit\"
```

**macOS**:
```bash
# Copy to OBS plugins folder
cp build/hoo-raid.so ~/Library/Application\ Support/obs-studio/plugins/hoo-raid/bin/
```

**Linux**:
```bash
# Copy to OBS plugins folder
cp build/hoo-raid.so ~/.config/obs-studio/plugins/hoo-raid/bin/64bit/
```

---

## Project Structure

```
HOO-Raid/
├── src/
│   ├── plugin.cpp           # Entry point
│   ├── ui/                   # Qt UI components
│   ├── services/             # Business logic
│   ├── models/               # Data structures
│   ├── storage/              # Persistence
│   └── i18n/                 # Translations
├── tests/                    # Unit & integration tests
├── cmake/                    # Build configuration
├── config/                   # Configuration templates
│   ├── secrets.example.h     # Template for credentials
│   └── secrets.local.h       # Your credentials (gitignored)
├── specs/                    # Feature specifications
└── CMakeLists.txt
```

---

## Development Workflow

### Running with Debug Output

1. Start OBS Studio
2. View → Docks → HOO-Raid (if not visible)
3. Check OBS log for debug output: Help → Log Files → View Current Log

### Making Changes

1. Create feature branch: `git checkout -b feature/your-feature`
2. Make changes
3. Build: `cmake --build build --config RelWithDebInfo`
4. Test in OBS
5. Run unit tests: `cd build && ctest`
6. Commit and push

### Testing OAuth Flow

For local development, the OAuth flow uses:
- Redirect URI: `http://localhost:3000/callback`
- A local HTTP server captures the callback

To test authentication:
1. Click "Login with Twitch" in the plugin
2. Browser opens Twitch authorization page
3. Authorize the application
4. Plugin receives the token automatically

---

## Common Tasks

### Add a New Translation

1. Copy `src/i18n/en-US.json` to new locale (e.g., `de-DE.json`)
2. Translate all strings
3. Add to CMakeLists.txt resource list
4. Rebuild

### Add a New Filter Field

1. Add field to `FilterPreset` struct in `models/filter-preset.h`
2. Update `matches()` method
3. Add UI control in `ui/filter-editor.cpp`
4. Update JSON serialization in `toJson()`/`fromJson()`
5. Add unit test in `tests/unit/test-preset-validation.cpp`

### Debug API Calls

Enable verbose logging in `services/twitch-api.cpp`:
```cpp
#define HOO_DEBUG_API 1
```

This logs all request/response data to OBS log.

---

## Build Verification

### Verify Successful Build

After building, verify the plugin compiled correctly:

**Check Binary Output**:
```bash
# Windows
dir build\RelWithDebInfo\hoo-raid.dll

# macOS
ls -la build/hoo-raid.so

# Linux
ls -la build/hoo-raid.so
```

**Verify Symbol Export** (Linux/macOS):
```bash
# Should show obs_module_load and other OBS entry points
nm -D build/hoo-raid.so | grep obs_module
```

**Verify Dependencies**:
```bash
# Linux
ldd build/hoo-raid.so

# macOS
otool -L build/hoo-raid.so
```

### Verify Plugin Loads in OBS

1. Copy plugin to OBS plugins folder (see Install for Testing above)
2. Launch OBS Studio
3. Check Help → Log Files → View Current Log for:
   ```
   [HOO-Raid] Plugin version 1.0.0 loading...
   [HOO-Raid] Plugin loaded successfully
   ```
4. Verify dock appears: View → Docks → HOO-Raid
5. If dock doesn't appear, check log for error messages

### Quick Smoke Test

1. Open HOO-Raid dock in OBS
2. Click "Login with Twitch" - browser should open
3. After auth, user name should appear in dock
4. Select "Beginner" preset
5. Click "Pick Random" - should show loading, then streamer preview
6. Click "Pick Another" - should show different streamer
7. Click "Logout" - should return to login screen

---

## Testing

### Run Unit Tests

```bash
cd build
ctest --output-on-failure
```

### Run Specific Test

```bash
cd build
./tests/unit/test-filter-logic
```

### Manual Testing Checklist

- [ ] Fresh install (no prior settings)
- [ ] OAuth login flow
- [ ] Each preset (Beginner, Small, Medium)
- [ ] Custom filter creation
- [ ] Greylist add/remove
- [ ] Re-roll functionality
- [ ] Raid execution
- [ ] Settings persist after OBS restart
- [ ] Token refresh after expiry
- [ ] Error handling (disconnect network, etc.)

---

## Troubleshooting

### Plugin Not Loading

1. Check OBS log for errors
2. Verify plugin is in correct folder
3. Check Qt version matches OBS build
4. Ensure all dependencies are present

### OAuth Callback Not Working

1. Verify redirect URI matches Twitch app settings exactly
2. Check local HTTP server started successfully
3. Check firewall isn't blocking localhost:3000

### API Rate Limits

- Twitch allows 800 requests/minute
- Raid endpoint: 10 per 10 minutes
- Implement caching to reduce API calls
- Check `Ratelimit-Remaining` header

### Build Errors

**Qt not found**:
```bash
cmake -DQt6_DIR=/path/to/qt/lib/cmake/Qt6 ..
```

**OBS SDK not found**:
```bash
cmake -Dlibobs_DIR=/path/to/obs-studio/build ..
```

---

## Resources

- [OBS Plugin Development Wiki](https://obsproject.com/wiki/Plugins)
- [OBS Plugin Template](https://github.com/obsproject/obs-plugintemplate)
- [Twitch API Documentation](https://dev.twitch.tv/docs/api/)
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [QtKeychain](https://github.com/frankosterfeld/qtkeychain)

---

## Getting Help

- Check existing issues on GitHub
- Join OBS Plugin Development Discord
- Review Twitch Developer Forums for API questions
