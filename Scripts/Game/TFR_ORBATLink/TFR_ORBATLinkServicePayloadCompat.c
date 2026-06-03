//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkServicePayloadCompat.c
// Compatibilidad de payload para mantener el contrato legacy y anadir ORBAT.
//
// No toca hooks, stats, medicina, kills, distancia, vehiculos ni explosivos.
// Solo restaura los campos de cabecera que ya usaba el backend:
// - token
// - scenario_id
// - scenario_name
// y mantiene los nuevos:
// - session_id
// - preset_id
// - map_name
// - players
// - markers

modded class TFR_ORBATLinkService
{
	override protected string BuildORBATPayload()
	{
		string payload = "{";

		payload += JsonString("token", m_Config.m_sBearerToken, true);
		payload += JsonString("scenario_id", m_Config.m_sScenarioId, true);
		payload += JsonString("scenario_name", m_Config.m_sScenarioName, true);

		payload += JsonString("session_id", m_Config.session_id, true);
		payload += JsonInt("preset_id", m_Config.preset_id, true);
		payload += JsonString("map_name", m_Config.m_sMapName, true);

		payload += "\"players\":[";

		bool first = true;

		foreach (int playerId, TFR_ORBATLinkPlayerStats stats : m_mPlayerStats)
		{
			if (!stats)
				continue;

			RefreshPlayerIdentity(playerId);
			UpdatePlayerRuntimeSnapshot(playerId);

			if (!first)
				payload += ",";

			payload += stats.ToPlayerJson();
			first = false;
		}

		payload += "],";
		payload += "\"markers\":[]";
		payload += "}";

		return payload;
	}
}
