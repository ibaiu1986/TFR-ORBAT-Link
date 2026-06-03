class TFR_ORBATLinkConfig : JsonApiStruct
{
	string m_sBaseUrl;
	string m_sRoute;
	string m_sBearerToken;

	// Nuevo contrato ORBAT externo.
	string session_id;
	int preset_id;

	// Legacy / escenario.
	string m_sMissionId;
	string m_sScenarioId;
	string m_sScenarioName;

	// Configuracion manual de mapa.
	int m_iPreformedMapId;
	string m_sMapName;

	bool m_bDebug;
	bool m_bSendOnDisconnect;
	bool m_bSendOnGameEnd;

	bool m_bSendTestOnRegister;
	int m_iSendTestDelayMs;

	bool m_bSendPeriodic;
	int m_iSendIntervalMinutes;

	void TFR_ORBATLinkConfig()
	{
		m_sBaseUrl = "https://tfr.gure.party";
		m_sRoute = "/wp-json/clan/v1/telemetry/push";
		m_sBearerToken = "";

		session_id = "";
		preset_id = 0;

		m_sMissionId = "";
		m_sScenarioId = "unknown";
		m_sScenarioName = "TFR ORBAT";

		m_iPreformedMapId = 0;
		m_sMapName = "unknown";

		m_bDebug = false;
		m_bSendOnDisconnect = true;
		m_bSendOnGameEnd = true;

		m_bSendTestOnRegister = false;
		m_iSendTestDelayMs = 5000;

		m_bSendPeriodic = true;
		m_iSendIntervalMinutes = 5;

		RegV("m_sBaseUrl");
		RegV("m_sRoute");
		RegV("m_sBearerToken");

		RegV("session_id");
		RegV("preset_id");

		RegV("m_sMissionId");
		RegV("m_sScenarioId");
		RegV("m_sScenarioName");

		RegV("m_iPreformedMapId");
		RegV("m_sMapName");

		RegV("m_bDebug");
		RegV("m_bSendOnDisconnect");
		RegV("m_bSendOnGameEnd");

		RegV("m_bSendTestOnRegister");
		RegV("m_iSendTestDelayMs");

		RegV("m_bSendPeriodic");
		RegV("m_iSendIntervalMinutes");
	}

	bool LoadConfig()
	{
		bool loaded = LoadFromFile("$profile:TFR_ORBATLink/config.json");

		if (!loaded)
		{
			Print("[TFR_ORBATLink] No se pudo cargar $profile:TFR_ORBATLink/config.json", LogLevel.ERROR);
			return false;
		}

		if (m_sBaseUrl.IsEmpty())
		{
			Print("[TFR_ORBATLink] Config invalida: m_sBaseUrl vacio", LogLevel.ERROR);
			return false;
		}

		if (m_sRoute.IsEmpty())
		{
			Print("[TFR_ORBATLink] Config invalida: m_sRoute vacio", LogLevel.ERROR);
			return false;
		}

		if (m_sBearerToken.IsEmpty())
		{
			Print("[TFR_ORBATLink] Config invalida: m_sBearerToken vacio", LogLevel.ERROR);
			return false;
		}

		if (session_id.IsEmpty())
		{
			Print("[TFR_ORBATLink] Config invalida: session_id vacio", LogLevel.ERROR);
			return false;
		}

		if (preset_id < 0)
		{
			Print("[TFR_ORBATLink] Config invalida: preset_id no puede ser negativo", LogLevel.ERROR);
			return false;
		}

		m_sScenarioId = TFR_ORBATLinkScenarioResolver.ResolveScenarioId();
		m_sScenarioName = TFR_ORBATLinkScenarioResolver.ResolveScenarioName();

		if (m_sScenarioId.IsEmpty())
			m_sScenarioId = "unknown";

		if (m_sScenarioName.IsEmpty())
			m_sScenarioName = "TFR ORBAT";

		if (m_sMapName.IsEmpty())
			m_sMapName = "unknown";

		if (m_iSendTestDelayMs < 1000)
			m_iSendTestDelayMs = 1000;

		if (m_iSendIntervalMinutes < 1)
			m_iSendIntervalMinutes = 1;

		if (m_bDebug)
		{
			Print(string.Format("[TFR_ORBATLink] Config cargada. baseUrl=%1 route=%2 tokenSet=%3 session_id=%4 preset_id=%5 scenario_id=%6 scenario_name=%7 map_name=%8 preformedMapId=%9",
				m_sBaseUrl,
				m_sRoute,
				!m_sBearerToken.IsEmpty(),
				session_id,
				preset_id,
				m_sScenarioId,
				m_sScenarioName,
				m_sMapName,
				m_iPreformedMapId
			), LogLevel.NORMAL);
		}

		return true;
	}
}
