# Contributing to HOO-RAID

First off, thank you for considering contributing to HOO-RAID! It's people like you that make this tool great for the streaming community.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Guidelines](#coding-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)

---

## Code of Conduct

This project and everyone participating in it is governed by our commitment to creating a welcoming, inclusive environment. By participating, you are expected to:

- Use welcoming and inclusive language
- Be respectful of differing viewpoints and experiences
- Gracefully accept constructive criticism
- Focus on what is best for the community
- Show empathy towards other community members

---

## Getting Started

### Prerequisites

Before you begin, ensure you have:

- OBS Studio 30.0+ installed
- A C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16+
- Qt 6.2+
- Git

### Fork and Clone

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/HOO-Raid.git
   cd HOO-Raid
   ```
3. Add the upstream remote:
   ```bash
   git remote add upstream https://github.com/ORIGINAL_OWNER/HOO-Raid.git
   ```

---

## How Can I Contribute?

### Reporting Bugs

Before creating a bug report, please check existing issues to avoid duplicates.

**When filing a bug report, include:**

1. **OBS Studio version** (Help → About)
2. **Operating System** and version
3. **HOO-RAID version**
4. **Steps to reproduce** the issue
5. **Expected behavior** vs **actual behavior**
6. **OBS log file** (Help → Log Files → Upload Current Log)
7. **Screenshots** if applicable

### Suggesting Features

Feature requests are welcome! Please:

1. Check if the feature has already been requested
2. Open an issue with the `enhancement` label
3. Clearly describe the feature and its benefits
4. Include mockups or examples if helpful

### Adding Translations

We welcome translations to make HOO-RAID accessible worldwide!

1. Copy `src/i18n/en-US.json` to your locale (e.g., `de-DE.json`)
2. Translate all string values (keep the keys in English)
3. Test your translation in OBS
4. Submit a pull request

**Current translations:**
- English (en-US) - Complete
- French (fr-FR) - Complete

**Needed translations:**
- German (de-DE)
- Spanish (es-ES)
- Portuguese (pt-BR)
- Japanese (ja-JP)
- Korean (ko-KR)
- And more!

### Code Contributions

Ready to write some code? Great! Here's how:

1. Find an issue to work on (or create one)
2. Comment on the issue to let others know you're working on it
3. Create a feature branch
4. Write your code
5. Test thoroughly
6. Submit a pull request

---

## Development Setup

### 1. Configure Twitch Credentials

```bash
cp config/secrets.example.h config/secrets.local.h
# Edit secrets.local.h with your Twitch API credentials
```

### 2. Build the Project

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### 3. Install for Testing

Copy the built plugin to your OBS plugins folder (see README for paths).

### 4. Enable Debug Logging

In `src/services/twitch-api.cpp`, set:
```cpp
#define HOO_DEBUG_API 1
```

---

## Coding Guidelines

### General Principles

- **Simplicity** - Prefer simple solutions over clever ones
- **Readability** - Code is read more often than written
- **Consistency** - Match existing code style

### C++ Style

```cpp
// Classes: PascalCase
class StreamerPreview : public QWidget {

// Methods: camelCase
void updateDisplay();

// Member variables: m_ prefix
QString m_userName;

// Constants: UPPER_SNAKE_CASE
static constexpr int MAX_RETRIES = 3;

// Local variables: camelCase
int retryCount = 0;
```

### File Organization

```
src/
├── models/       # Data structures (Streamer, FilterPreset, etc.)
├── services/     # Business logic (TwitchApi, OAuthManager, etc.)
├── storage/      # Persistence (SettingsManager, CredentialStore)
├── ui/           # Qt widgets (RaidPickerDock, StreamerPreview)
├── i18n/         # Translation files
└── plugin.cpp    # Entry point
```

### Qt Conventions

- Use Qt's parent-child ownership model
- Prefer signals/slots over callbacks
- Use `Q_OBJECT` macro for QObject subclasses
- Follow Qt naming conventions for signals/slots

### Logging

Use OBS logging functions:

```cpp
blog(LOG_INFO, "[HOO-Raid] Informational message");
blog(LOG_WARNING, "[HOO-Raid] Warning message");
blog(LOG_ERROR, "[HOO-Raid] Error message");
blog(LOG_DEBUG, "[HOO-Raid] Debug message");
```

### Error Handling

- Never silently swallow errors
- Log errors with context
- Provide user-friendly error messages
- Use Qt's signal/slot for async error propagation

---

## Commit Messages

We follow conventional commit format:

```
type(scope): short description

Longer description if needed.

Closes #123
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style (formatting, no logic change)
- `refactor`: Code restructuring
- `test`: Adding tests
- `chore`: Build process, dependencies

**Examples:**
```
feat(filters): add category autocomplete to filter editor

fix(oauth): handle token refresh on 401 response

docs(readme): add macOS installation instructions

refactor(api): extract rate limit handling to helper class
```

---

## Pull Request Process

### Before Submitting

1. **Sync with upstream:**
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Run tests:**
   ```bash
   cd build && ctest --output-on-failure
   ```

3. **Test in OBS:**
   - Plugin loads without errors
   - Your changes work as expected
   - No regressions in existing features

4. **Update documentation** if needed

### Submitting

1. Push your branch to your fork
2. Open a pull request against `main`
3. Fill out the PR template completely
4. Link related issues

### PR Template

```markdown
## Description
[Describe your changes]

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing Done
- [ ] Tested on Windows
- [ ] Tested on macOS
- [ ] Tested on Linux
- [ ] Added unit tests

## Screenshots
[If applicable]

## Related Issues
Closes #

## Checklist
- [ ] Code follows project style
- [ ] Self-reviewed my code
- [ ] Added comments for complex logic
- [ ] Updated documentation
- [ ] No new warnings
```

### Review Process

1. Maintainers will review your PR
2. Address any requested changes
3. Once approved, your PR will be merged
4. Your contribution will be credited in release notes

---

## Questions?

- Open a [Discussion](https://github.com/YOUR_USERNAME/HOO-Raid/discussions)
- Check existing issues and discussions
- Reach out to maintainers

Thank you for contributing to HOO-RAID!
