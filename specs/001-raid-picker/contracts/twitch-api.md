# Twitch API Contracts

**Feature**: 001-raid-picker
**API Version**: Helix (current)
**Base URL**: `https://api.twitch.tv/helix`

## Authentication

All requests require:
- `Authorization: Bearer {access_token}`
- `Client-Id: {client_id}`

---

## Endpoints

### 1. Get Streams

Fetches live streams with optional filters.

**Request**:
```
GET /streams
```

**Query Parameters**:
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| game_id | string | No | Filter by game ID |
| language | string | No | Filter by ISO 639-1 language code |
| first | integer | No | Page size (1-100, default 20) |
| after | string | No | Cursor for next page |

**Response** (200 OK):
```json
{
    "data": [
        {
            "id": "41375541868",
            "user_id": "123456789",
            "user_login": "streamer_name",
            "user_name": "Streamer_Name",
            "game_id": "509658",
            "game_name": "Just Chatting",
            "type": "live",
            "title": "Stream title here",
            "viewer_count": 42,
            "started_at": "2026-01-11T10:00:00Z",
            "language": "en",
            "thumbnail_url": "https://static-cdn.jtvnw.net/previews-ttv/live_user_streamer_name-{width}x{height}.jpg",
            "tag_ids": [],
            "tags": ["English", "Chill"],
            "is_mature": false
        }
    ],
    "pagination": {
        "cursor": "eyJiIjp7IkN..."
    }
}
```

**Error Responses**:
| Code | Description |
|------|-------------|
| 400 | Invalid parameter |
| 401 | Invalid/expired token |
| 429 | Rate limit exceeded |

---

### 2. Get Followed Streams

Fetches live streams from channels the user follows.

**Request**:
```
GET /streams/followed?user_id={user_id}
```

**Query Parameters**:
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| user_id | string | Yes | Must match OAuth token user |
| first | integer | No | Page size (1-100, default 100) |
| after | string | No | Cursor for next page |

**Required Scope**: `user:read:follows`

**Response** (200 OK):
```json
{
    "data": [
        {
            "id": "41375541868",
            "user_id": "987654321",
            "user_login": "followed_streamer",
            "user_name": "Followed_Streamer",
            "game_id": "509658",
            "game_name": "Just Chatting",
            "type": "live",
            "title": "Friendly stream!",
            "viewer_count": 150,
            "started_at": "2026-01-11T08:00:00Z",
            "language": "fr",
            "thumbnail_url": "https://...",
            "tag_ids": [],
            "tags": ["French"]
        }
    ],
    "pagination": {
        "cursor": "..."
    }
}
```

---

### 3. Get Channel Followers

Gets follower count for a channel.

**Request**:
```
GET /channels/followers?broadcaster_id={broadcaster_id}
```

**Query Parameters**:
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| broadcaster_id | string | Yes | Channel to query |
| first | integer | No | Page size (max 100) |

**Response** (200 OK):
```json
{
    "total": 12345,
    "data": [],
    "pagination": {}
}
```

**Note**: Without `moderator:read:followers` scope, only `total` is returned (which is sufficient for our needs).

---

### 4. Get Games

Resolves game names to IDs for filtering.

**Request**:
```
GET /games?name={game_name}
```

**Query Parameters**:
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| name | string | No | Game name (exact match) |
| id | string | No | Game ID |

**Response** (200 OK):
```json
{
    "data": [
        {
            "id": "509658",
            "name": "Just Chatting",
            "box_art_url": "https://..."
        }
    ]
}
```

---

### 5. Start a Raid

Initiates a raid to another channel.

**Request**:
```
POST /raids?from_broadcaster_id={from_id}&to_broadcaster_id={to_id}
```

**Query Parameters**:
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| from_broadcaster_id | string | Yes | Raiding channel (must match token) |
| to_broadcaster_id | string | Yes | Target channel |

**Required Scope**: `channel:manage:raids`

**Response** (200 OK):
```json
{
    "data": [
        {
            "created_at": "2026-01-11T14:30:00Z",
            "is_mature": false
        }
    ]
}
```

**Error Responses**:
| Code | Description |
|------|-------------|
| 400 | Invalid parameters or cannot raid self |
| 401 | Invalid/expired token |
| 404 | Target broadcaster not found |
| 429 | Rate limit (10 per 10 min) |

**Important**: Response confirms raid is *queued*, not executed. User has 90 seconds to confirm in Twitch UI.

---

### 6. Cancel a Raid

Cancels a pending raid.

**Request**:
```
DELETE /raids?broadcaster_id={broadcaster_id}
```

**Query Parameters**:
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| broadcaster_id | string | Yes | Must match token user |

**Required Scope**: `channel:manage:raids`

**Response** (204 No Content): Success

**Error Responses**:
| Code | Description |
|------|-------------|
| 401 | Invalid/expired token |
| 404 | No active raid to cancel |

---

## Rate Limits

| Endpoint | Limit |
|----------|-------|
| General | 800 requests/minute |
| Start Raid | 10 requests/10 minutes |

**Handling 429**:
- Check `Ratelimit-Reset` header for retry time
- Implement exponential backoff
- Show user-friendly message

---

## OAuth Flow

### Authorization URL
```
https://id.twitch.tv/oauth2/authorize
    ?client_id={client_id}
    &redirect_uri={redirect_uri}
    &response_type=code
    &scope=user:read:follows+channel:manage:raids
```

### Token Exchange
```
POST https://id.twitch.tv/oauth2/token
Content-Type: application/x-www-form-urlencoded

client_id={client_id}
&client_secret={client_secret}
&code={authorization_code}
&grant_type=authorization_code
&redirect_uri={redirect_uri}
```

### Token Refresh
```
POST https://id.twitch.tv/oauth2/token
Content-Type: application/x-www-form-urlencoded

client_id={client_id}
&client_secret={client_secret}
&grant_type=refresh_token
&refresh_token={refresh_token}
```

### Token Response
```json
{
    "access_token": "...",
    "refresh_token": "...",
    "expires_in": 14400,
    "scope": ["user:read:follows", "channel:manage:raids"],
    "token_type": "bearer"
}
```
