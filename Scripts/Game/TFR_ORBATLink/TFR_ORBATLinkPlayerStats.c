class TFR_ORBATLinkPlayerStats
{
	int m_iPlayerId;

	string m_sBohemiaUid;
	string m_sSteamId64;
	string m_sPlayerName;

	string m_sScenarioId;
	string m_sScenarioName;

	int m_iPeriodStartMs;

	int m_iMissionsPlayed;

	int m_iKills;
	int m_iDeaths;
	int m_iFriendlyFire;

	int m_iShotsFired;
	int m_iShotsHit;

	int m_iMedicalBandagesApplied;
	int m_iMedicalTourniquetsApplied;
	int m_iMedicalSalineApplied;
	int m_iMedicalMorphineApplied;
	int m_iMedicalEpinephrineApplied;

	int m_iDistanceWalkedM;
	int m_iDistanceInVehicleM;

	int m_iVehiclesDestroyedTotal;
	int m_iVehiclesDestroyedLight;
	int m_iVehiclesDestroyedHeavy;
	int m_iVehiclesDestroyedAir;
	int m_iVehiclesDestroyedSea;
	int m_iVehiclesDestroyedStatic;

	int m_iPlacedExplosivesDetonated;

	// Campos del contrato ORBAT nuevo.
	string m_sFaction;
	string m_sSquad;
	string m_sRole;
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;
	float m_fHeading;
	float m_fSpeedKmh;
	bool m_bIsAlive;

	// GPS / posicion legacy. Se conserva por compatibilidad interna/debug.
	int m_iEjex;
	int m_iEjey;
	int m_iDir;

	void TFR_ORBATLinkPlayerStats(int playerId)
	{
		m_iPlayerId = playerId;

		m_sBohemiaUid = "";
		m_sSteamId64 = "";
		m_sPlayerName = "";

		m_sScenarioId = "unknown";
		m_sScenarioName = "TFR ORBAT";

		m_iPeriodStartMs = System.GetTickCount();

		m_iMissionsPlayed = 1;

		m_iKills = 0;
		m_iDeaths = 0;
		m_iFriendlyFire = 0;

		m_iShotsFired = 0;
		m_iShotsHit = 0;

		m_iMedicalBandagesApplied = 0;
		m_iMedicalTourniquetsApplied = 0;
		m_iMedicalSalineApplied = 0;
		m_iMedicalMorphineApplied = 0;
		m_iMedicalEpinephrineApplied = 0;

		m_iDistanceWalkedM = 0;
		m_iDistanceInVehicleM = 0;

		m_iVehiclesDestroyedTotal = 0;
		m_iVehiclesDestroyedLight = 0;
		m_iVehiclesDestroyedHeavy = 0;
		m_iVehiclesDestroyedAir = 0;
		m_iVehiclesDestroyedSea = 0;
		m_iVehiclesDestroyedStatic = 0;

		m_iPlacedExplosivesDetonated = 0;

		m_sFaction = "unknown";
		m_sSquad = "";
		m_sRole = "";
		m_fPosX = 0.0;
		m_fPosY = 0.0;
		m_fPosZ = 0.0;
		m_fHeading = 0.0;
		m_fSpeedKmh = 0.0;
		m_bIsAlive = true;

		m_iEjex = 0;
		m_iEjey = 0;
		m_iDir = 0;
	}

	int GetPeriodMinutes()
	{
		int now = System.GetTickCount();
		int elapsedMs = now - m_iPeriodStartMs;

		if (elapsedMs <= 0)
			return 0;

		float minutes = elapsedMs / 60000.0;
		return Math.Round(minutes);
	}

	int GetDistanceTotalM()
	{
		return m_iDistanceWalkedM + m_iDistanceInVehicleM;
	}

	void ResetPeriod()
	{
		m_iPeriodStartMs = System.GetTickCount();

		// No se resetea m_iMissionsPlayed.
		m_iKills = 0;
		m_iDeaths = 0;
		m_iFriendlyFire = 0;

		m_iShotsFired = 0;
		m_iShotsHit = 0;

		m_iMedicalBandagesApplied = 0;
		m_iMedicalTourniquetsApplied = 0;
		m_iMedicalSalineApplied = 0;
		m_iMedicalMorphineApplied = 0;
		m_iMedicalEpinephrineApplied = 0;

		m_iDistanceWalkedM = 0;
		m_iDistanceInVehicleM = 0;

		m_iVehiclesDestroyedTotal = 0;
		m_iVehiclesDestroyedLight = 0;
		m_iVehiclesDestroyedHeavy = 0;
		m_iVehiclesDestroyedAir = 0;
		m_iVehiclesDestroyedSea = 0;
		m_iVehiclesDestroyedStatic = 0;

		m_iPlacedExplosivesDetonated = 0;

		// No se resetean identidad ni posicion.
	}

	bool HasAnyStats()
	{
		if (m_iMissionsPlayed > 0)
			return true;

		if (GetPeriodMinutes() > 0)
			return true;

		if (m_iKills > 0)
			return true;

		if (m_iDeaths > 0)
			return true;

		if (m_iShotsFired > 0)
			return true;

		if (m_iShotsHit > 0)
			return true;

		if (m_iMedicalBandagesApplied > 0)
			return true;

		if (m_iMedicalTourniquetsApplied > 0)
			return true;

		if (m_iMedicalSalineApplied > 0)
			return true;

		if (m_iMedicalMorphineApplied > 0)
			return true;

		if (m_iMedicalEpinephrineApplied > 0)
			return true;

		if (m_iDistanceWalkedM > 0)
			return true;

		if (m_iDistanceInVehicleM > 0)
			return true;

		if (m_iVehiclesDestroyedTotal > 0)
			return true;

		if (m_iPlacedExplosivesDetonated > 0)
			return true;

		return false;
	}

	string CleanJsonValue(string value)
	{
		if (value.IsEmpty())
			return "";

		value.Replace("\\", "/");
		value.Replace("\"", "'");
		value.Replace("\n", " ");
		value.Replace("\r", " ");
		value.Replace("\t", " ");

		return value;
	}

	string JsonStringField(string key, string value, bool comma = true)
	{
		string result = "\"" + key + "\":\"" + CleanJsonValue(value) + "\"";

		if (comma)
			result += ",";

		return result;
	}

	string JsonIntField(string key, int value, bool comma = true)
	{
		string result = "\"" + key + "\":" + value.ToString();

		if (comma)
			result += ",";

		return result;
	}

	string JsonFloatField(string key, float value, bool comma = true)
	{
		string result = "\"" + key + "\":" + value.ToString();

		if (comma)
			result += ",";

		return result;
	}

	string JsonBoolField(string key, bool value, bool comma = true)
	{
		string boolText = "false";

		if (value)
			boolText = "true";

		string result = "\"" + key + "\":" + boolText;

		if (comma)
			result += ",";

		return result;
	}

	string ToPlayerJson()
	{
		string steamId = m_sSteamId64;

		if (steamId.IsEmpty())
			steamId = m_sBohemiaUid;

		string json = "{";

		// Identidad y ORBAT vivo.
		json += JsonStringField("steamid", steamId);
		json += JsonStringField("bohemia_uid", m_sBohemiaUid);
		json += JsonStringField("name", m_sPlayerName);
		json += JsonStringField("faction", m_sFaction);
		json += JsonStringField("squad", m_sSquad);
		json += JsonStringField("role", m_sRole);
		json += JsonFloatField("pos_x", m_fPosX);
		json += JsonFloatField("pos_y", m_fPosY);
		json += JsonFloatField("pos_z", m_fPosZ);
		json += JsonFloatField("heading", m_fHeading);
		json += JsonFloatField("speed_kmh", m_fSpeedKmh);
		json += JsonBoolField("is_alive", m_bIsAlive);

		// Dossier de combate del operador. Se mantiene todo lo que ya usaba la web.
		// missions_played se omite porque lo gestiona otro sistema.
		json += JsonIntField("kills", m_iKills);
		json += JsonIntField("deaths", m_iDeaths);
		json += JsonIntField("friendly_fire", m_iFriendlyFire);
		json += JsonIntField("shots_fired", m_iShotsFired);
		json += JsonIntField("shots_hit", m_iShotsHit);
		json += JsonIntField("playtime_minutes", GetPeriodMinutes());

		json += JsonIntField("medical_bandages_applied", m_iMedicalBandagesApplied);
		json += JsonIntField("medical_tourniquets_applied", m_iMedicalTourniquetsApplied);
		json += JsonIntField("medical_saline_applied", m_iMedicalSalineApplied);
		json += JsonIntField("medical_morphine_applied", m_iMedicalMorphineApplied);
		json += JsonIntField("medical_epinephrine_applied", m_iMedicalEpinephrineApplied);

		json += JsonIntField("distance_walked_m", m_iDistanceWalkedM);
		json += JsonIntField("distance_in_vehicle_m", m_iDistanceInVehicleM);
		json += JsonIntField("distance_total_m", GetDistanceTotalM());

		json += JsonIntField("vehicles_destroyed_total", m_iVehiclesDestroyedTotal);
		json += JsonIntField("vehicles_destroyed_light", m_iVehiclesDestroyedLight);
		json += JsonIntField("vehicles_destroyed_heavy", m_iVehiclesDestroyedHeavy);
		json += JsonIntField("vehicles_destroyed_air", m_iVehiclesDestroyedAir);
		json += JsonIntField("vehicles_destroyed_sea", m_iVehiclesDestroyedSea);
		json += JsonIntField("vehicles_destroyed_static", m_iVehiclesDestroyedStatic);

		json += JsonIntField("placed_explosives_detonated", m_iPlacedExplosivesDetonated);

		// Campos legacy que ya existian para mapa/compatibilidad.
		json += JsonIntField("Ejex", m_iEjex);
		json += JsonIntField("Ejey", m_iEjey);
		json += JsonIntField("Dir", m_iDir, false);

		json += "}";

		return json;
	}
}
