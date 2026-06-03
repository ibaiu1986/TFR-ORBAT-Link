# TFR ORBAT Link Implementation Checklist

This checklist tracks the current implementation target for TFR ORBAT Link.

## 1. Configuration loader

Create a runtime config loader that reads from the server profile:

```text
profile/config.json
```

Required fields:

```json
{
  "session_id": "string",
  "preset_id": 0
}
```

Validation rules:

- `session_id` must exist and must not be empty.
- `preset_id` must exist and should be a valid integer.
- missing config must produce a clear log message.
- invalid config must produce a clear log message.
- the addon should fail gracefully instead of crashing.

Suggested internal structure:

```c
class TFR_ORBATLinkConfig
{
    string session_id;
    int preset_id;
}
```

## 2. Payload classes

Suggested payload structure:

```c
class TFR_ORBATLinkPayload
{
    string session_id;
    int preset_id;
    string map_name;
    ref array<ref TFR_ORBATLinkPlayerPayload> players;
    ref array<ref TFR_ORBATLinkMarkerPayload> markers;
}
```

```c
class TFR_ORBATLinkPlayerPayload
{
    string steamid;
    string name;
    string faction;
    string squad;
    string role;
    float pos_x;
    float pos_y;
    float pos_z;
    float heading;
    float speed_kmh;
    bool is_alive;
}
```

```c
class TFR_ORBATLinkMarkerPayload
{
    string id;
    string type;
    string label;
    float pos_x;
    float pos_y;
    string reported_by;
    string color;
}
```

## 3. Payload builder

The payload builder should:

- load `session_id` and `preset_id` from config;
- resolve `map_name`;
- collect player telemetry;
- collect marker/objective telemetry;
- always include `players` and `markers` arrays;
- serialize the final payload to JSON.

Important:

- Do not send `m_iPreformedMapId` to the endpoint.
- Send `map_name` instead.
- Build one complete payload, not isolated player-only packets.

## 4. HTTP sender

The sender should:

- send the JSON payload to the configured endpoint;
- log success responses;
- log endpoint errors;
- log serialization errors;
- avoid debug spam during normal operation;
- allow send interval/rate limiting if periodic telemetry is used.

## 5. Modularity rules

The addon must remain modular:

- optional dependencies must not be hard requirements unless absolutely necessary;
- missing optional integrations must not break compilation;
- optional systems should be disabled safely when unavailable;
- shared/base telemetry should continue working even if one optional module is missing.

## 6. Server safety

The implementation should avoid:

- heavy global scans every frame;
- duplicate `CallLater` loops;
- excessive network spam;
- uncontrolled debug logging;
- unbounded marker/player processing.

## 7. Minimum acceptance test

A valid runtime should be able to produce and send this shape:

```json
{
  "session_id": "example-session-id",
  "preset_id": 0,
  "map_name": "everon",
  "players": [],
  "markers": []
}
```

Then progressively fill `players` and `markers` with runtime data.
