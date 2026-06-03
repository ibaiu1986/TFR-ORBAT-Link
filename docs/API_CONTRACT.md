# TFR ORBAT Link API Contract

This document describes the current payload expected by the external ORBAT endpoint.

## Top-level payload

```json
{
  "session_id": "string",
  "preset_id": 0,
  "map_name": "everon",
  "players": [
    {
      "steamid": "76561198...",
      "name": "Jugador",
      "faction": "BLUFOR",
      "squad": "Alpha-1",
      "role": "Rifleman",
      "pos_x": 5420.5,
      "pos_y": 3210.3,
      "pos_z": 15.2,
      "heading": 270,
      "speed_kmh": 5,
      "is_alive": true
    }
  ],
  "markers": [
    {
      "id": "obj1",
      "type": "objective",
      "label": "Capturar base",
      "pos_x": 5000,
      "pos_y": 3000,
      "reported_by": "",
      "color": "#FFB000"
    }
  ]
}
```

## Required top-level fields

| Field | Type | Source | Notes |
| --- | --- | --- | --- |
| `session_id` | string | `profile/config.json` | External session identifier. Must not be hardcoded. |
| `preset_id` | integer | `profile/config.json` | External preset identifier. Must not be hardcoded. |
| `map_name` | string | Game/runtime or configured map name | Example: `everon`. |
| `players` | array | Runtime player telemetry | Can be empty if no players are available. |
| `markers` | array | Runtime marker/objective telemetry | Can be empty if no markers are available. |

## Player object

| Field | Type | Notes |
| --- | --- | --- |
| `steamid` | string | Player Steam ID where available. |
| `name` | string | Player display name. |
| `faction` | string | Faction label, for example `BLUFOR`. |
| `squad` | string | Squad/group label, for example `Alpha-1`. |
| `role` | string | Role label, for example `Rifleman`. |
| `pos_x` | number | World position X. |
| `pos_y` | number | World position Y. |
| `pos_z` | number | World position Z/elevation. |
| `heading` | number | Direction in degrees. |
| `speed_kmh` | number | Current speed in km/h. |
| `is_alive` | boolean | Player alive/dead state. |

## Marker object

| Field | Type | Notes |
| --- | --- | --- |
| `id` | string | Stable marker/objective identifier. |
| `type` | string | Marker type, for example `objective`. |
| `label` | string | Human-readable marker label. |
| `pos_x` | number | World position X. |
| `pos_y` | number | World position Y. |
| `reported_by` | string | Reporter/player/source name. Empty string is valid. |
| `color` | string | Hex color, for example `#FFB000`. |

## Important notes

- `session_id` and `preset_id` must be loaded from `profile/config.json`.
- `m_iPreformedMapId` must not be sent to the endpoint.
- The endpoint expects a complete payload, not isolated player updates.
- Arrays should be sent even when empty.
- Logs should clearly report missing config, invalid config, payload build errors, send errors, and successful responses.
