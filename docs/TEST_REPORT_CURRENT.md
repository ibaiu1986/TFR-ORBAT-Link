# TFR ORBATLink - Current server test findings

Date: 2026-06-03

## Confirmed by server test

The current payload/update flow is partially working after restoring the combined legacy + ORBAT contract.

Working or apparently working:

- Payload is being sent.
- New player ORBAT fields are being collected when a player is registered and spawned.
- Some counters are being collected in the running server, including shots, distance and vehicle destruction.

## Latest server log diagnosis

The latest uploaded server log shows two different states:

1. Before player registration, the periodic sender posts an empty payload with `players: []` and the web rejects it with HTTP 400.
2. After player registration and spawn, the payload contains the player, but the web returns HTTP 500 WordPress critical error.

Important evidence from the log:

- The running server payload still includes fields that were removed from the current GitHub version:
  - `scenario_name`
  - `friendly_fire`
  - `Ejex`
  - `Ejey`
  - `Dir`
- Therefore the server test was not running the latest corrected script set from the repository, or an old local/workshop copy is still being loaded.
- The current GitHub `TFR_ORBATLinkService.c` no longer writes `scenario_name` to the root payload.
- The current GitHub `TFR_ORBATLinkPlayerStats.c` no longer writes `friendly_fire`, `Ejex`, `Ejey` or `Dir` to the player payload.

Scenario note:

- The log resolves `scenario_id` as `{6FDBF9AB994098D9}Missions/World_ORbat.conf` and derives `scenario_name` as `World ORbat`.
- The web screenshot showing another scenario is probably stale data from the last successful stored match because the new POST requests are failing with HTTP 500 and are not being saved.

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
- First verify that the server is actually loading the current GitHub scripts before changing medical or kill hooks.
