<!--
  =============================================================================
  SYNC IMPACT REPORT
  =============================================================================
  Version change: N/A (initial) → 1.0.0

  Modified principles: N/A (initial creation)

  Added sections:
    - Core Principles (5 principles defined)
    - Technical Constraints
    - Development Workflow
    - Governance

  Removed sections: N/A

  Templates validation:
    ✅ .specify/templates/plan-template.md - Constitution Check section compatible
    ✅ .specify/templates/spec-template.md - Requirements structure compatible
    ✅ .specify/templates/tasks-template.md - Phase structure compatible
    ✅ .specify/templates/checklist-template.md - No conflicts
    ✅ .specify/templates/agent-file-template.md - No conflicts

  Follow-up TODOs: None
  =============================================================================
-->

# HOO-Raid Constitution

## Core Principles

### I. Simplicity First (YAGNI)

- Every feature MUST solve a concrete, immediate user need
- No speculative features: if it is not required now, do not build it
- Prefer the simplest solution that works over elaborate abstractions
- Before adding complexity, document why simpler alternatives are insufficient
- Remove unused code immediately; do not comment it out "for later"

**Rationale**: OBS plugins run in a performance-sensitive streaming environment.
Unnecessary complexity increases bugs, maintenance burden, and resource usage.

### II. OBS Integration Standards

- All UI components MUST follow OBS plugin conventions and theming
- Plugin MUST NOT block the OBS main thread during API calls
- Twitch API calls MUST be asynchronous with proper error handling
- Plugin state MUST persist across OBS restarts via OBS data settings
- All user-facing text MUST support localization

**Rationale**: Streamers depend on OBS stability; a misbehaving plugin
can disrupt live broadcasts and damage user trust.

### III. Privacy & Security

- User OAuth tokens MUST be stored securely, never logged or exposed
- Greylist data MUST remain local to the user's machine
- No telemetry or analytics without explicit user consent
- API requests MUST use HTTPS exclusively
- Follow Twitch API terms of service for all integrations

**Rationale**: Streamers trust the plugin with their Twitch credentials
and followed streamer data; this trust must not be violated.

### IV. User Experience

- Random raid selection MUST be previewable before execution
- Filter presets (Beginner/Small/Medium) MUST have clear, documented criteria
- Error states MUST provide actionable feedback to the user
- Plugin MUST gracefully degrade when Twitch API is unavailable
- All actions MUST be reversible or confirmable where applicable

**Rationale**: Raids are public, visible actions; users need confidence
in their selections and the ability to review before committing.

### V. Maintainability

- Code MUST be readable without extensive comments; prefer clear naming
- External dependencies MUST be justified and version-pinned
- Breaking changes to filter logic MUST be documented in release notes
- Configuration schema changes MUST include migration handling
- Build and test instructions MUST be kept current in documentation

**Rationale**: Open-source projects thrive on contributor accessibility;
low friction for new contributors ensures project longevity.

## Technical Constraints

- **Platform**: OBS Studio plugin (obs-websocket or native C/C++ plugin)
- **External API**: Twitch Helix API for streamer discovery and raid execution
- **Storage**: OBS settings API for configuration persistence
- **Authentication**: Twitch OAuth 2.0 with appropriate scopes
- **UI Framework**: OBS-native Qt widgets or browser source panel

## Development Workflow

- All features begin with a specification (`/speckit.specify`)
- Implementation plans precede coding (`/speckit.plan`)
- Tasks are tracked systematically (`/speckit.tasks`)
- Manual testing required before release (OBS environment)
- Releases follow semantic versioning (MAJOR.MINOR.PATCH)

## Governance

- This constitution supersedes conflicting practices or ad-hoc decisions
- Amendments require:
  1. Written proposal with rationale
  2. Review of impact on existing features
  3. Version bump per semantic versioning rules
- Complexity additions MUST be justified in the Complexity Tracking table
- All contributors MUST review this constitution before significant changes

**Version**: 1.0.0 | **Ratified**: 2026-01-11 | **Last Amended**: 2026-01-11
