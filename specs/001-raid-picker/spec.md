# Feature Specification: Raid Picker Plugin

**Feature Branch**: `001-raid-picker`
**Created**: 2026-01-11
**Status**: Draft
**Input**: User description: "OBS plugin to allow streamers pick and raid another streamer based on various search filters (Language, Category/Game, viewers, followers), with 3 presets (Beginner/Small/Medium), greylist support, followed streamers access, and preview before raid execution"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Quick Raid with Preset (Priority: P1)

As a streamer ending my broadcast, I want to quickly find and raid a smaller streamer using a preset filter so I can support the community without spending time configuring filters manually.

**Why this priority**: This is the core value proposition - enabling quick, filtered raid discovery. Without this, the plugin has no purpose.

**Independent Test**: Can be fully tested by selecting a preset, clicking "Pick Random", reviewing the result, and executing the raid. Delivers immediate value of finding raid targets.

**Acceptance Scenarios**:

1. **Given** the streamer is authenticated with Twitch, **When** they select the "Beginner" preset and click "Pick Random", **Then** the system displays a streamer matching the preset criteria (e.g., low follower count, small viewer count)
2. **Given** a random streamer has been picked, **When** the streamer clicks "Preview", **Then** they see the target's stream title, category, viewer count, follower count, and language
3. **Given** the streamer has previewed a pick, **When** they click "Launch Raid", **Then** the raid is initiated on their Twitch channel
4. **Given** the streamer has previewed a pick, **When** they click "Pick Another", **Then** a new random streamer is selected matching the same criteria

---

### User Story 2 - Custom Filter Raid (Priority: P2)

As a streamer with specific preferences, I want to configure custom filters to find streamers that match my exact criteria (language, category, viewer range, follower range).

**Why this priority**: Extends the core functionality for power users who want more control beyond presets.

**Independent Test**: Can be tested by setting custom filters, picking a random streamer, and verifying the result matches all specified criteria.

**Acceptance Scenarios**:

1. **Given** the plugin is open, **When** the streamer sets language to "French", category to "Just Chatting", viewers between 5-50, **Then** only streamers matching ALL these criteria are eligible for random selection
2. **Given** custom filters are set, **When** no streamers match the criteria, **Then** the system displays a clear message indicating no matches found with suggestions to broaden filters
3. **Given** a filter configuration exists, **When** the streamer saves it as a custom preset, **Then** it appears in the preset list for future use

---

### User Story 3 - Raid Followed Streamers (Priority: P3)

As a streamer who wants to support my network, I want to access my followed streamers list and randomly pick from those who are currently live.

**Why this priority**: Provides an alternative raid source for streamers who prefer supporting known channels.

**Independent Test**: Can be tested by loading followed streamers list, applying optional filters, and selecting a random live followed streamer.

**Acceptance Scenarios**:

1. **Given** the streamer is authenticated, **When** they select "Raid from Followed", **Then** the system shows only followed streamers who are currently live
2. **Given** the followed list is displayed, **When** the streamer applies filters (e.g., category), **Then** only matching followed streamers remain in the selection pool
3. **Given** a followed streamer is picked, **When** the streamer previews them, **Then** the same preview information is shown as with discovery raids

---

### User Story 4 - Greylist Management (Priority: P4)

As a streamer who has had negative experiences with certain channels, I want to greylist streamers so they never appear in my raid suggestions.

**Why this priority**: Important for user safety and experience, but the plugin can function without it initially.

**Independent Test**: Can be tested by adding a streamer to the greylist and verifying they no longer appear in random picks.

**Acceptance Scenarios**:

1. **Given** a streamer preview is displayed, **When** the user clicks "Add to Greylist", **Then** that streamer is added to the greylist and immediately removed from the current selection pool
2. **Given** streamers are on the greylist, **When** a random pick is made, **Then** greylisted streamers are never included in results
3. **Given** the greylist contains entries, **When** the user opens greylist management, **Then** they see all greylisted streamers with options to remove them
4. **Given** a greylisted streamer is removed, **When** the next random pick occurs, **Then** that streamer is eligible again

---

### User Story 5 - Persistent Settings (Priority: P5)

As a streamer who uses OBS regularly, I want my authentication, greylist, and custom presets to persist across OBS restarts.

**Why this priority**: Quality-of-life feature that enhances usability but is not required for core functionality.

**Independent Test**: Can be tested by configuring settings, restarting OBS, and verifying all settings are restored.

**Acceptance Scenarios**:

1. **Given** the user has authenticated with Twitch, **When** OBS is restarted, **Then** the authentication persists (unless token expired)
2. **Given** custom presets have been saved, **When** OBS is restarted, **Then** all custom presets are available
3. **Given** streamers have been greylisted, **When** OBS is restarted, **Then** the greylist is preserved

---

### Edge Cases

- What happens when the Twitch API is unavailable or rate-limited?
  - System displays a friendly error message and allows retry
- What happens when the OAuth token expires mid-session?
  - System prompts re-authentication without losing greylist/preset data
- What happens when all eligible streamers are greylisted?
  - System informs user no streamers match criteria, suggests clearing greylist or broadening filters
- What happens when a picked streamer goes offline before raid launch?
  - System detects the change during preview and prompts to pick another
- What happens when the user has no followed streamers who are live?
  - System displays message indicating no followed streamers are currently live

## Requirements *(mandatory)*

### Functional Requirements

#### Authentication & Authorization
- **FR-001**: System MUST authenticate users via Twitch OAuth 2.0 with appropriate scopes for raid execution
- **FR-002**: System MUST securely store OAuth tokens and refresh them as needed
- **FR-003**: System MUST allow users to disconnect/re-authenticate at any time

#### Streamer Discovery
- **FR-004**: System MUST fetch live streamers from Twitch based on filter criteria
- **FR-005**: System MUST support filtering by: language, category/game, viewer count range, follower count range
- **FR-006**: System MUST provide three built-in presets: Beginner, Small, Medium with predefined filter values
- **FR-007**: System MUST allow users to create, edit, and delete custom filter presets
- **FR-008**: System MUST fetch the user's followed streamers who are currently live

#### Raid Selection
- **FR-009**: System MUST randomly select one streamer from the filtered pool
- **FR-010**: System MUST exclude greylisted streamers from all selection pools
- **FR-011**: System MUST allow users to re-roll (pick another random streamer) without re-querying

#### Preview & Execution
- **FR-012**: System MUST display a preview showing: stream title, category, current viewers, follower count, language, and time streaming
- **FR-013**: System MUST allow users to launch the raid from the preview screen
- **FR-014**: System MUST confirm raid success or display error if raid fails

#### Greylist Management
- **FR-015**: System MUST allow users to add streamers to greylist from preview or search
- **FR-016**: System MUST allow users to view and manage (remove entries from) the greylist
- **FR-017**: System MUST persist greylist data locally across OBS sessions

#### Data Persistence
- **FR-018**: System MUST persist authentication state across OBS restarts
- **FR-019**: System MUST persist custom presets across OBS restarts
- **FR-020**: System MUST persist greylist across OBS restarts

### Assumptions

- Users have a Twitch account with broadcaster permissions
- Users are streaming via OBS when using the raid functionality
- Twitch API rate limits are sufficient for typical usage patterns (a few searches per stream end)
- The three presets have the following criteria:
  - **Beginner**: 0-100 followers, 1-5 viewers
  - **Small**: 100-1000 followers, 5-25 viewers
  - **Medium**: 1000-10000 followers, 25-100 viewers

### Key Entities

- **Streamer**: A Twitch user currently live streaming; attributes include username, display name, stream title, category, language, viewer count, follower count, stream start time
- **Filter Preset**: A named configuration of filter criteria; can be built-in (Beginner/Small/Medium) or user-defined
- **Greylist Entry**: A record of a streamer the user has chosen to exclude; stores streamer ID and display name
- **Session State**: Current authentication status, active filters, and last picked streamer

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Users can discover and raid a streamer in under 30 seconds using a preset
- **SC-002**: 95% of raid attempts successfully execute when the target is still live
- **SC-003**: Filter results return within 3 seconds under normal network conditions
- **SC-004**: Users can add a streamer to the greylist in a single click from preview
- **SC-005**: All user settings (auth, presets, greylist) persist correctly across 100% of OBS restarts
- **SC-006**: Plugin does not increase OBS startup time by more than 1 second
- **SC-007**: Plugin never blocks OBS main thread during network operations
