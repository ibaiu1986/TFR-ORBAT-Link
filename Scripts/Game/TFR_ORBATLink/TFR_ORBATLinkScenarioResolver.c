//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkScenarioResolver.c
// Resolucion minima del escenario para ORBATLink.
//
// No toca ACE.
// No toca medicina.
// No toca acciones.
// No usa SCR_MissionHeader.m_sName porque en algunas misiones devuelve "<Insert world name>".
// scenario_name se deriva de scenario_id.
//
// Ejemplo:
// {6FDBF9AB994098D9}Missions/World_ORbat.conf
// -> World ORbat

class TFR_ORBATLinkScenarioResolver
{
	static string ResolveScenarioId()
	{
		ChimeraGame game = ChimeraGame.Cast(GetGame());

		if (!game)
			return "unknown";

		MissionHeader missionHeader = game.GetMissionHeader();

		if (!missionHeader)
			return "unknown";

		ResourceName headerResourceName = missionHeader.GetHeaderResourceName();
		string scenarioId = headerResourceName;

		if (scenarioId.IsEmpty())
			return "unknown";

		Print("[TFR_ORBATLink] ScenarioResolver: scenario_id resuelto=" + scenarioId, LogLevel.NORMAL);

		return scenarioId;
	}

	static string ResolveScenarioName()
	{
		string scenarioId = ResolveScenarioId();

		if (scenarioId.IsEmpty() || scenarioId == "unknown")
			return "TFR ORBAT";

		string scenarioName = scenarioId;

		int idx = scenarioName.IndexOf("Missions/");

		if (idx >= 0)
		{
			idx = idx + 9;

			if (idx < scenarioName.Length())
				scenarioName = scenarioName.Substring(idx, scenarioName.Length() - idx);
		}

		scenarioName.Replace(".conf", "");
		scenarioName.Replace("_", " ");
		scenarioName.Replace("/", " ");

		if (scenarioName.IsEmpty())
			return "TFR ORBAT";

		if (scenarioName == "<Insert world name>")
			return "TFR ORBAT";

		if (scenarioName.Contains("Insert world name"))
			return "TFR ORBAT";

		Print("[TFR_ORBATLink] ScenarioResolver: scenario_name derivado=" + scenarioName, LogLevel.NORMAL);

		return scenarioName;
	}
}
