# TFR ORBATLink - Current server test findings

Date: 2026-06-03

## Confirmed by server test

The current payload/update flow is partially working after restoring the combined legacy + ORBAT contract.

Working or apparently working:

- Payload is being sent.
- Most statistics appear to refresh on the web.
- The legacy dossier fields are present in the player payload.
- The new ORBAT fields are present in the player payload.

## Current failures reported from server test

Reported as not working:

- Tourniquet usage is not reflected on the web.
- Saline usage is not reflected on the web.
- Kills/deaths/KD did not change after killing a BLUFOR player.

Notes:

- The BLUFOR kill may need separate handling depending on whether the victim was a player, AI, same faction/friendly fire, or whether the GameMode OnPlayerKilled hook fired.
- Tourniquet and saline failures must be fixed without changing working systems.
- Do not touch unrelated hooks or counters until the log confirms the failing path.

## Contract that must be preserved

Root payload:

```json
{
  "token": "...",
  "scenario_id": "...",
  "session_id": "...",
  "preset_id": 0,
  "map_name": "everon",
  "players": [],
  "markers": []
}
```

Player payload must preserve:

```text
steamid
bohemia_uid
name
faction
squad
role
pos_x
pos_y
pos_z
heading
speed_kmh
is_alive
kills
deaths
shots_fired
shots_hit
playtime_minutes
medical_bandages_applied
medical_tourniquets_applied
medical_saline_applied
medical_morphine_applied
medical_epinephrine_applied
distance_walked_m
distance_in_vehicle_m
distance_total_m
vehicles_destroyed_total
vehicles_destroyed_light
vehicles_destroyed_heavy
vehicles_destroyed_air
vehicles_destroyed_sea
vehicles_destroyed_static
placed_explosives_detonated
```

## Fix policy

- No blind rewrites.
- No deleting working code.
- Scripts must be delivered complete with path.
- Fix only the failing paths: tourniquet, saline, and kill/death/KD.
