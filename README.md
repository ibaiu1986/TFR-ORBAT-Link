# TFR ORBAT Link

TFR ORBAT Link is a telemetry addon for the Task Force Ratas community. Its goal is to send live Arma Reforger session data to an external ORBAT/web platform.

The addon is designed around a simple and stable payload contract:

- runtime session configuration is read from the server profile;
- player telemetry is collected in-game;
- marker/objective telemetry is collected in-game;
- a complete JSON payload is sent to the configured external endpoint.

## Current implementation target

The endpoint expects one complete payload containing:

- `session_id`
- `preset_id`
- `map_name`
- `players`
- `markers`

`session_id` and `preset_id` must be loaded from `profile/config.json`. They must not be hardcoded in the addon.

`m_iPreformedMapId` is considered local/manual configuration only and must not be sent to the endpoint. The payload sends `map_name` instead.

## Expected runtime config

The server profile should contain a `config.json` file with at least:

```json
{
  "session_id": "string",
  "preset_id": 0
}
```

See [`docs/examples/config.example.json`](docs/examples/config.example.json).

## Payload contract

The current API contract is documented in:

- [`docs/API_CONTRACT.md`](docs/API_CONTRACT.md)

## Implementation checklist

The current technical checklist is documented in:

- [`docs/IMPLEMENTATION_CHECKLIST.md`](docs/IMPLEMENTATION_CHECKLIST.md)

## Modularity rule

This addon must remain modular:

- optional dependencies must only affect the system they enable;
- missing optional dependencies must not break compilation;
- the base addon must remain usable without optional integrations;
- configuration errors should produce clear logs instead of hard failures where possible.

## License

This project is licensed for non-commercial use only.

You may use, modify, and share this addon for personal, community, and non-commercial purposes.

Commercial use is not allowed without prior written permission from the copyright holder.
