//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkScenarioResolver.c
// Resolucion minima del escenario para ORBATLink.
//
// No toca ACE.
// No toca medicina.
// No toca acciones.
// No usa SCR_MissionHeader.m_sName porque en algunas misiones devuelve "<Insert world name>".
//
// El backend/web esta usando scenario_id como texto visible de escenario.
// Por eso scenario_id debe enviarse limpio para visualizacion:
// {6FDBF9AB994098D9}Missions/World_ORbat.conf
// -> World ORbat

class TFR_ORBATLinkScenarioResolver
{
	static string ResolveScenarioId()
	{
		string scenarioName = ResolveScenarioNameFromHeaderResource();

		if (scenarioName.IsEmpty())
			scenarioName = "TFR ORBAT";

		Print("[TFR_ORBATLink] ScenarioResolver: scenario_id visible=" + scenarioName, LogLevel.NORMAL);

		return scenarioName;
	}

	static string ResolveScenarioName()
	{
		string scenarioName = ResolveScenarioNameFromHeaderResource();

		if (scenarioName.IsEmpty())
			scenarioName = "TFR ORBAT";

		Print("[TFR_ORBATLink] ScenarioResolver: scenario_name derivado=" + scenarioName, LogLevel.NORMAL);

		return scenarioName;
	}

	protected static string ResolveScenarioNameFromHeaderResource()
	{
		ChimeraGame game = ChimeraGame.Cast(GetGame());

		if (!game)
			return "TFR ORBAT";

		MissionHeader missionHeader = game.GetMissionHeader();

		if (!missionHeader)
			return "TFR ORBAT";

		ResourceName headerResourceName = missionHeader.GetHeaderResourceName();
		string scenarioName = headerResourceName;

		if (scenarioName.IsEmpty())
			return "TFR ORBAT";

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

		return scenarioName;
	}
}
