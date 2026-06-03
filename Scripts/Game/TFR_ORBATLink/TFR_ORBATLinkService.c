class TFR_ORBATLinkService
{
	protected static ref TFR_ORBATLinkService s_Instance;

	protected ref TFR_ORBATLinkConfig m_Config;
	protected ref map<int, ref TFR_ORBATLinkPlayerStats> m_mPlayerStats;
	protected ref array<ref TFR_ORBATLinkRestCallback> m_aCallbacks;
	protected ref map<string, int> m_mRecentMedicalEvents;
	protected ref map<string, int> m_mRecentShotHitEvents;

	protected ref map<int, bool> m_mPendingAutoSend;
	protected ref map<int, int> m_mLastAutoSendMs;
	protected ref map<int, vector> m_mLastSpeedPositions;
	protected ref map<int, int> m_mLastSpeedSampleMs;

	protected RestContext m_RestContext;

	protected bool m_bInitialized;
	protected bool m_bEnabled;

	protected const int TFR_AUTOSEND_DEFAULT_DELAY_MS = 3000;
	protected const int TFR_AUTOSEND_MIN_INTERVAL_MS = 20000;
	protected const int TFR_SHOT_HIT_DEDUPE_MS = 50;

	static TFR_ORBATLinkService GetInstance()
	{
		if (!s_Instance)
			s_Instance = new TFR_ORBATLinkService();

		return s_Instance;
	}

	void TFR_ORBATLinkService()
	{
		m_mPlayerStats = new map<int, ref TFR_ORBATLinkPlayerStats>();
		m_aCallbacks = new array<ref TFR_ORBATLinkRestCallback>();
		m_mRecentMedicalEvents = new map<string, int>();
		m_mRecentShotHitEvents = new map<string, int>();

		m_mPendingAutoSend = new map<int, bool>();
		m_mLastAutoSendMs = new map<int, int>();
		m_mLastSpeedPositions = new map<int, vector>();
		m_mLastSpeedSampleMs = new map<int, int>();

		m_bInitialized = false;
		m_bEnabled = false;
	}

	void Init()
	{
		if (m_bInitialized)
			return;

		m_bInitialized = true;

		if (!Replication.IsServer())
			return;

		m_Config = new TFR_ORBATLinkConfig();

		if (!m_Config.LoadConfig())
		{
			Print("[TFR_ORBATLink] Servicio desactivado por config invalida.", LogLevel.ERROR);
			return;
		}

		RestApi restApi = GetGame().GetRestApi();

		if (!restApi)
		{
			Print("[TFR_ORBATLink] GetRestApi() devolvio null.", LogLevel.ERROR);
			return;
		}

		m_RestContext = restApi.GetContext(m_Config.m_sBaseUrl);

		if (!m_RestContext)
		{
			Print("[TFR_ORBATLink] No se pudo crear RestContext.", LogLevel.ERROR);
			return;
		}

		m_RestContext.SetHeaders("Content-Type: application/json");

		m_bEnabled = true;

		Print("[TFR_ORBATLink] Servicio iniciado.", LogLevel.NORMAL);

		if (m_Config.m_bSendPeriodic)
		{
			int intervalMs = m_Config.m_iSendIntervalMinutes * 60000;
			GetGame().GetCallqueue().CallLater(SendPeriodicTick, intervalMs, true);

			Print("[TFR_ORBATLink] Envio periodico activado cada " + m_Config.m_iSendIntervalMinutes.ToString() + " minutos.", LogLevel.NORMAL);
		}
	}

	void OnPlayerRegistered(int playerId)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (m_mPlayerStats.Contains(playerId))
		{
			RefreshPlayerIdentity(playerId);
			UpdatePlayerRuntimeSnapshot(playerId);
			MarkPlayerActiveForVehicleAttribution(playerId);
			return;
		}

		ref TFR_ORBATLinkPlayerStats stats = new TFR_ORBATLinkPlayerStats(playerId);

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (playerManager)
			stats.m_sPlayerName = playerManager.GetPlayerName(playerId);

		stats.m_sScenarioId = m_Config.m_sScenarioId;
		stats.m_sScenarioName = m_Config.m_sScenarioName;

		m_mPlayerStats.Insert(playerId, stats);

		RefreshPlayerIdentity(playerId);
		UpdatePlayerRuntimeSnapshot(playerId);
		MarkPlayerActiveForVehicleAttribution(playerId);

		if (m_Config.m_bDebug)
		{
			Print(string.Format("[TFR_ORBATLink] Registrado playerId=%1 name=%2 uid=%3 steamid=%4 session_id=%5 preset_id=%6 map_name=%7",
				playerId,
				stats.m_sPlayerName,
				stats.m_sBohemiaUid,
				stats.m_sSteamId64,
				m_Config.session_id,
				m_Config.preset_id,
				m_Config.m_sMapName
			), LogLevel.NORMAL);
		}

		GetGame().GetCallqueue().CallLater(RefreshPlayerIdentity, 5000, false, playerId);
		GetGame().GetCallqueue().CallLater(RefreshPlayerIdentity, 15000, false, playerId);

		GetGame().GetCallqueue().CallLater(ScheduleAutoSend, 8000, false, playerId);

		if (m_Config.m_bSendTestOnRegister)
		{
			Print("[TFR_ORBATLink] TEST POST programado para playerId=" + playerId.ToString(), LogLevel.WARNING);
			GetGame().GetCallqueue().CallLater(SendPlayerStatsById, m_Config.m_iSendTestDelayMs, false, playerId);
		}
	}

	void RefreshPlayerIdentity(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (!m_mPlayerStats.Contains(playerId))
			return;

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		BackendApi backendApi = GetGame().GetBackendApi();

		if (playerManager)
		{
			string playerName = playerManager.GetPlayerName(playerId);

			if (!playerName.IsEmpty())
				stats.m_sPlayerName = playerName;
		}

		if (!backendApi)
			return;

		string identityId = backendApi.GetPlayerIdentityId(playerId);
		string playerUid = backendApi.GetPlayerUID(playerId);
		string platformId = backendApi.GetPlayerPlatformId(playerId);

		if (!identityId.IsEmpty())
			stats.m_sBohemiaUid = identityId;
		else if (!playerUid.IsEmpty())
			stats.m_sBohemiaUid = playerUid;

		if (!platformId.IsEmpty())
			stats.m_sSteamId64 = platformId;

		if (m_Config && m_Config.m_bDebug)
		{
			Print(string.Format("[TFR_ORBATLink] Identity refresh playerId=%1 name=%2 uid=%3 steamid=%4",
				playerId,
				stats.m_sPlayerName,
				stats.m_sBohemiaUid,
				stats.m_sSteamId64
			), LogLevel.NORMAL);
		}
	}

	void OnPlayerDisconnected(int playerId)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		Print("[TFR_ORBATLink] OnPlayerDisconnected recibido playerId=" + playerId.ToString(), LogLevel.NORMAL);

		if (!m_mPlayerStats.Contains(playerId))
			return;

		RefreshPlayerIdentity(playerId);
		UpdatePlayerRuntimeSnapshot(playerId);

		if (m_Config.m_bSendOnDisconnect)
			SendBatchStats(false);

		ClearAutoSendState(playerId);

		m_mPlayerStats.Remove(playerId);

		if (m_mLastSpeedPositions && m_mLastSpeedPositions.Contains(playerId))
			m_mLastSpeedPositions.Remove(playerId);

		if (m_mLastSpeedSampleMs && m_mLastSpeedSampleMs.Contains(playerId))
			m_mLastSpeedSampleMs.Remove(playerId);
	}

	void OnPlayerKilled(int victimPlayerId, IEntity victimEntity, IEntity killerEntity, notnull Instigator killer)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (victimPlayerId <= 0)
			return;

		if (!m_mPlayerStats.Contains(victimPlayerId))
			OnPlayerRegistered(victimPlayerId);

		TFR_ORBATLinkPlayerStats victimStats = m_mPlayerStats.Get(victimPlayerId);

		if (victimStats)
		{
			victimStats.m_iDeaths++;
			victimStats.m_bIsAlive = false;
		}

		int killerPlayerId = ResolveKillerPlayerId(killerEntity, killer);

		if (killerPlayerId <= 0)
		{
			ScheduleAutoSend(victimPlayerId);
			return;
		}

		if (killerPlayerId == victimPlayerId)
		{
			ScheduleAutoSend(victimPlayerId);
			return;
		}

		if (!m_mPlayerStats.Contains(killerPlayerId))
			OnPlayerRegistered(killerPlayerId);

		RefreshPlayerIdentity(killerPlayerId);
		UpdatePlayerRuntimeSnapshot(killerPlayerId);

		TFR_ORBATLinkPlayerStats killerStats = m_mPlayerStats.Get(killerPlayerId);

		if (!killerStats)
			return;

		killerStats.m_iKills++;

		if (IsFriendlyFire(killerPlayerId, victimEntity))
			killerStats.m_iFriendlyFire++;

		MarkPlayerActiveForVehicleAttribution(killerPlayerId);
		ScheduleAutoSend(killerPlayerId);
		ScheduleAutoSend(victimPlayerId);

		if (m_Config && m_Config.m_bDebug)
		{
			Print(string.Format("[TFR_ORBATLink] Baja contra jugador registrada. killer=%1 victim=%2 kills=%3",
				killerPlayerId,
				victimPlayerId,
				killerStats.m_iKills
			), LogLevel.NORMAL);
		}
	}

	void OnAIKilled(int killerPlayerId)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (killerPlayerId <= 0)
			return;

		if (!m_mPlayerStats.Contains(killerPlayerId))
			OnPlayerRegistered(killerPlayerId);

		RefreshPlayerIdentity(killerPlayerId);
		UpdatePlayerRuntimeSnapshot(killerPlayerId);

		TFR_ORBATLinkPlayerStats killerStats = m_mPlayerStats.Get(killerPlayerId);

		if (!killerStats)
			return;

		killerStats.m_iKills++;

		MarkPlayerActiveForVehicleAttribution(killerPlayerId);
		ScheduleAutoSend(killerPlayerId);

		if (m_Config && m_Config.m_bDebug)
		{
			Print(string.Format("[TFR_ORBATLink] Baja contra IA registrada. killer=%1 kills=%2",
				killerPlayerId,
				killerStats.m_iKills
			), LogLevel.NORMAL);
		}
	}

	void OnShotFired(int playerId)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (!m_mPlayerStats.Contains(playerId))
			OnPlayerRegistered(playerId);

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		stats.m_iShotsFired++;

		TFR_ORBATLinkVehicleDamageHelper.RegisterRecentCombatPlayer(playerId);

		if (m_Config && m_Config.m_bDebug)
		{
			Print(string.Format("[TFR_ORBATLink] Disparo registrado. playerId=%1 shots_fired=%2",
				playerId,
				stats.m_iShotsFired
			), LogLevel.NORMAL);
		}
	}

	void OnShotHit(int playerId, IEntity targetEntity)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (!targetEntity)
			return;

		if (!m_mPlayerStats.Contains(playerId))
			OnPlayerRegistered(playerId);

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		string targetKey = targetEntity.ToString();
		string dedupeKey = playerId.ToString() + "_" + targetKey;
		int now = System.GetTickCount();

		if (!m_mRecentShotHitEvents)
			m_mRecentShotHitEvents = new map<string, int>();

		if (m_mRecentShotHitEvents.Contains(dedupeKey))
		{
			int lastHit = m_mRecentShotHitEvents.Get(dedupeKey);

			if (now - lastHit < TFR_SHOT_HIT_DEDUPE_MS)
				return;

			m_mRecentShotHitEvents.Remove(dedupeKey);
		}

		m_mRecentShotHitEvents.Insert(dedupeKey, now);

		stats.m_iShotsHit++;

		MarkPlayerActiveForVehicleAttribution(playerId);
		ScheduleAutoSend(playerId);

		if (m_Config && m_Config.m_bDebug)
		{
			Print(string.Format("[TFR_ORBATLink] Impacto registrado. playerId=%1 shots_hit=%2 target=%3",
				playerId,
				stats.m_iShotsHit,
				targetEntity
			), LogLevel.NORMAL);
		}
	}

	void OnDistanceWalked(int playerId, int meters)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (meters <= 0)
			return;

		if (!m_mPlayerStats.Contains(playerId))
			OnPlayerRegistered(playerId);

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		stats.m_iDistanceWalkedM += meters;

		MarkPlayerActiveForVehicleAttribution(playerId);
	}

	void OnDistanceInVehicle(int playerId, int meters)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (meters <= 0)
			return;

		if (!m_mPlayerStats.Contains(playerId))
			OnPlayerRegistered(playerId);

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		stats.m_iDistanceInVehicleM += meters;

		MarkPlayerActiveForVehicleAttribution(playerId);
	}

	void OnVehicleDestroyed(int playerId, string vehicleCategory)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (!m_mPlayerStats.Contains(playerId))
			OnPlayerRegistered(playerId);

		RefreshPlayerIdentity(playerId);
		UpdatePlayerRuntimeSnapshot(playerId);

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		stats.m_iVehiclesDestroyedTotal++;

		if (vehicleCategory == "light")
			stats.m_iVehiclesDestroyedLight++;
		else if (vehicleCategory == "heavy")
			stats.m_iVehiclesDestroyedHeavy++;
		else if (vehicleCategory == "air")
			stats.m_iVehiclesDestroyedAir++;
		else if (vehicleCategory == "sea")
			stats.m_iVehiclesDestroyedSea++;
		else if (vehicleCategory == "static")
			stats.m_iVehiclesDestroyedStatic++;

		MarkPlayerActiveForVehicleAttribution(playerId);
		ScheduleAutoSend(playerId);
	}

	void OnPlacedExplosiveDetonated(int playerId)
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (!m_mPlayerStats.Contains(playerId))
			OnPlayerRegistered(playerId);

		UpdatePlayerRuntimeSnapshot(playerId);

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		stats.m_iPlacedExplosivesDetonated++;

		MarkPlayerActiveForVehicleAttribution(playerId);
		ScheduleAutoSend(playerId);
	}

	void OnMedicalConsumableUsed(int playerId, SCR_EConsumableType typeId)
	{
		string medicalCode = ResolveMedicalCodeFromType(typeId);
		OnMedicalConsumableUsedByCode(playerId, medicalCode, typeId.ToString(), "");
	}

	void OnMedicalConsumableUsedByCode(int playerId, string medicalCode, string typeName = "", string effectTypeName = "")
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (medicalCode.IsEmpty())
			return;

		if (!m_mPlayerStats.Contains(playerId))
			OnPlayerRegistered(playerId);

		UpdatePlayerRuntimeSnapshot(playerId);

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		string dedupeKey = playerId.ToString() + "_" + medicalCode;
		int now = System.GetTickCount();

		if (m_mRecentMedicalEvents && m_mRecentMedicalEvents.Contains(dedupeKey))
		{
			int lastTime = m_mRecentMedicalEvents.Get(dedupeKey);

			if (now - lastTime < 3000)
				return;
		}

		if (m_mRecentMedicalEvents)
		{
			if (m_mRecentMedicalEvents.Contains(dedupeKey))
				m_mRecentMedicalEvents.Remove(dedupeKey);

			m_mRecentMedicalEvents.Insert(dedupeKey, now);
		}

		if (medicalCode == "BANDAGE")
			stats.m_iMedicalBandagesApplied++;
		else if (medicalCode == "TOURNIQUET")
			stats.m_iMedicalTourniquetsApplied++;
		else if (medicalCode == "SALINE")
			stats.m_iMedicalSalineApplied++;
		else if (medicalCode == "MORPHINE")
			stats.m_iMedicalMorphineApplied++;
		else if (medicalCode == "EPINEPHRINE")
			stats.m_iMedicalEpinephrineApplied++;
		else
			return;

		MarkPlayerActiveForVehicleAttribution(playerId);
		ScheduleAutoSend(playerId);
	}

	protected string ResolveMedicalCodeFromType(SCR_EConsumableType typeId)
	{
		switch (typeId)
		{
			case SCR_EConsumableType.BANDAGE:
				return "BANDAGE";

			case SCR_EConsumableType.TOURNIQUET:
				return "TOURNIQUET";

			case SCR_EConsumableType.SALINE:
				return "SALINE";

			case SCR_EConsumableType.MORPHINE:
				return "MORPHINE";
		}

		string typeName = typeId.ToString();

		if (typeName == "ACE_MEDICAL_EPINEPHRINE")
			return "EPINEPHRINE";

		if (typeName.Contains("EPINEPHRINE"))
			return "EPINEPHRINE";

		if (typeName == "7")
			return "EPINEPHRINE";

		if (typeName.Contains("Value is 7"))
			return "EPINEPHRINE";

		return "";
	}

	protected void UpdatePlayerRuntimeSnapshot(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		if (!m_mPlayerStats.Contains(playerId))
			return;

		TFR_ORBATLinkPlayerStats stats = m_mPlayerStats.Get(playerId);

		if (!stats)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return;

		IEntity controlledEntity = playerManager.GetPlayerControlledEntity(playerId);

		if (!controlledEntity)
		{
			stats.m_bIsAlive = false;
			stats.m_fSpeedKmh = 0.0;
			return;
		}

		vector origin = controlledEntity.GetOrigin();

		stats.m_fPosX = origin[0];
		stats.m_fPosY = origin[2];
		stats.m_fPosZ = origin[1];

		stats.m_iEjex = Math.Round(origin[0]);
		stats.m_iEjey = Math.Round(origin[2]);

		vector angles = controlledEntity.GetAngles();
		float yaw = angles[0];

		while (yaw < 0)
			yaw += 360.0;

		while (yaw >= 360.0)
			yaw -= 360.0;

		int dir = Math.Round(yaw);

		if (dir >= 360)
			dir = 0;

		if (dir < 0)
			dir = 0;

		stats.m_fHeading = dir;
		stats.m_iDir = dir;

		stats.m_fSpeedKmh = CalculateSpeedKmh(playerId, origin);
		stats.m_bIsAlive = ResolveIsAlive(controlledEntity);
		stats.m_sFaction = ResolvePlayerFaction(playerId, controlledEntity);
		stats.m_sSquad = ResolvePlayerSquad(playerId);
		stats.m_sRole = ResolvePlayerRole(controlledEntity);
	}

	protected float CalculateSpeedKmh(int playerId, vector currentPos)
	{
		if (!m_mLastSpeedPositions)
			m_mLastSpeedPositions = new map<int, vector>();

		if (!m_mLastSpeedSampleMs)
			m_mLastSpeedSampleMs = new map<int, int>();

		int now = System.GetTickCount();
		float speedKmh = 0.0;

		if (m_mLastSpeedPositions.Contains(playerId) && m_mLastSpeedSampleMs.Contains(playerId))
		{
			vector lastPos = m_mLastSpeedPositions.Get(playerId);
			int lastMs = m_mLastSpeedSampleMs.Get(playerId);
			int elapsedMs = now - lastMs;

			if (elapsedMs > 0)
			{
				float distanceM = vector.Distance(lastPos, currentPos);
				float seconds = elapsedMs / 1000.0;
				speedKmh = (distanceM / seconds) * 3.6;
			}
		}

		if (m_mLastSpeedPositions.Contains(playerId))
			m_mLastSpeedPositions.Remove(playerId);

		if (m_mLastSpeedSampleMs.Contains(playerId))
			m_mLastSpeedSampleMs.Remove(playerId);

		m_mLastSpeedPositions.Insert(playerId, currentPos);
		m_mLastSpeedSampleMs.Insert(playerId, now);

		if (speedKmh < 0.0)
			speedKmh = 0.0;

		if (speedKmh > 300.0)
			speedKmh = 0.0;

		return speedKmh;
	}

	protected bool ResolveIsAlive(IEntity controlledEntity)
	{
		if (!controlledEntity)
			return false;

		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));

		if (!damageManager)
			return true;

		return damageManager.GetState() != EDamageState.DESTROYED;
	}

	protected string ResolvePlayerFaction(int playerId, IEntity controlledEntity)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (factionManager)
		{
			Faction playerFaction = factionManager.GetPlayerFaction(playerId);

			if (playerFaction)
				return playerFaction.GetFactionKey();
		}

		if (controlledEntity)
		{
			FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));

			if (factionComponent)
			{
				Faction entityFaction = factionComponent.GetAffiliatedFaction();

				if (entityFaction)
					return entityFaction.GetFactionKey();
			}
		}

		return "unknown";
	}

	protected string ResolvePlayerSquad(int playerId)
	{
		return "";
	}

	protected string ResolvePlayerRole(IEntity controlledEntity)
	{
		if (!controlledEntity)
			return "";

		EntityPrefabData prefabData = controlledEntity.GetPrefabData();

		if (!prefabData)
			return "";

		string prefabName = prefabData.GetPrefabName();
		prefabName.Replace(".et", "");
		prefabName.Replace(".conf", "");

		return prefabName;
	}

	protected void MarkPlayerActiveForVehicleAttribution(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		TFR_ORBATLinkVehicleDamageHelper.RegisterRecentActivePlayer(playerId);
	}

	protected void ScheduleAutoSend(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (!m_bEnabled)
			return;

		if (playerId <= 0)
			return;

		if (!m_mPendingAutoSend)
			m_mPendingAutoSend = new map<int, bool>();

		if (!m_mLastAutoSendMs)
			m_mLastAutoSendMs = new map<int, int>();

		if (m_mPendingAutoSend.Contains(playerId))
			return;

		int delayMs = TFR_AUTOSEND_DEFAULT_DELAY_MS;
		int now = System.GetTickCount();

		if (m_mLastAutoSendMs.Contains(playerId))
		{
			int lastSend = m_mLastAutoSendMs.Get(playerId);
			int elapsed = now - lastSend;

			if (elapsed < TFR_AUTOSEND_MIN_INTERVAL_MS)
				delayMs = (TFR_AUTOSEND_MIN_INTERVAL_MS - elapsed) + TFR_AUTOSEND_DEFAULT_DELAY_MS;
		}

		m_mPendingAutoSend.Insert(playerId, true);

		GetGame().GetCallqueue().CallLater(FlushAutoSend, delayMs, false, playerId);

		if (m_Config && m_Config.m_bDebug)
			Print("[TFR_ORBATLink] AutoSend programado playerId=" + playerId.ToString() + " delayMs=" + delayMs.ToString(), LogLevel.NORMAL);
	}

	protected void FlushAutoSend(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (!m_bEnabled)
			return;

		if (m_mPendingAutoSend && m_mPendingAutoSend.Contains(playerId))
			m_mPendingAutoSend.Remove(playerId);

		if (!m_mPlayerStats.Contains(playerId))
			return;

		RefreshPlayerIdentity(playerId);
		UpdatePlayerRuntimeSnapshot(playerId);

		SendBatchStats(true);

		if (!m_mLastAutoSendMs)
			m_mLastAutoSendMs = new map<int, int>();

		if (m_mLastAutoSendMs.Contains(playerId))
			m_mLastAutoSendMs.Remove(playerId);

		m_mLastAutoSendMs.Insert(playerId, System.GetTickCount());

		if (m_Config && m_Config.m_bDebug)
			Print("[TFR_ORBATLink] AutoSend ejecutado playerId=" + playerId.ToString(), LogLevel.NORMAL);
	}

	protected void ClearAutoSendState(int playerId)
	{
		if (m_mPendingAutoSend && m_mPendingAutoSend.Contains(playerId))
			m_mPendingAutoSend.Remove(playerId);

		if (m_mLastAutoSendMs && m_mLastAutoSendMs.Contains(playerId))
			m_mLastAutoSendMs.Remove(playerId);
	}

	protected int ResolveKillerPlayerId(IEntity killerEntity, notnull Instigator killer)
	{
		int killerPlayerId = killer.GetInstigatorPlayerID();

		if (killerPlayerId > 0)
			return killerPlayerId;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return 0;

		if (killerEntity)
		{
			killerPlayerId = playerManager.GetPlayerIdFromControlledEntity(killerEntity);

			if (killerPlayerId > 0)
				return killerPlayerId;
		}

		IEntity instigatorEntity = killer.GetInstigatorEntity();

		if (instigatorEntity)
		{
			killerPlayerId = playerManager.GetPlayerIdFromControlledEntity(instigatorEntity);

			if (killerPlayerId > 0)
				return killerPlayerId;
		}

		return 0;
	}

	void SendAll()
	{
		if (!Replication.IsServer())
			return;

		Init();

		if (!m_bEnabled)
			return;

		if (!m_Config.m_bSendOnGameEnd)
			return;

		SendBatchStats(false);
	}

	void SendPeriodicTick()
	{
		if (!Replication.IsServer())
			return;

		if (!m_bEnabled)
			return;

		SendBatchStats(true);
	}

	void SendPlayerStatsById(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (!m_bEnabled)
			return;

		if (!m_mPlayerStats.Contains(playerId))
		{
			Print("[TFR_ORBATLink] TEST POST cancelado. Player no encontrado: " + playerId.ToString(), LogLevel.WARNING);
			return;
		}

		RefreshPlayerIdentity(playerId);
		UpdatePlayerRuntimeSnapshot(playerId);

		Print("[TFR_ORBATLink] TEST POST ejecutando para playerId=" + playerId.ToString(), LogLevel.WARNING);

		SendBatchStats(true);
	}

	protected bool IsFriendlyFire(int killerPlayerId, IEntity victimEntity)
	{
		if (!victimEntity)
			return false;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (!factionManager)
			return false;

		Faction killerFaction = factionManager.GetPlayerFaction(killerPlayerId);

		if (!killerFaction)
			return false;

		FactionAffiliationComponent victimFactionComponent = FactionAffiliationComponent.Cast(victimEntity.FindComponent(FactionAffiliationComponent));

		if (!victimFactionComponent)
			return false;

		Faction victimFaction = victimFactionComponent.GetAffiliatedFaction();

		if (!victimFaction)
			return false;

		return killerFaction.IsFactionFriendly(victimFaction);
	}

	protected string CleanJsonValue(string value)
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

	protected string JsonString(string key, string value, bool comma = true)
	{
		string result = "\"" + key + "\":\"" + CleanJsonValue(value) + "\"";

		if (comma)
			result += ",";

		return result;
	}

	protected string JsonInt(string key, int value, bool comma = true)
	{
		string result = "\"" + key + "\":" + value.ToString();

		if (comma)
			result += ",";

		return result;
	}

	protected void SendBatchStats(bool resetAfterSend)
	{
		string payload = BuildORBATPayload();

		if (payload.IsEmpty())
			return;

		SendPayload(payload, resetAfterSend);
	}

	protected void ResetAllPeriods()
	{
		foreach (int playerId, TFR_ORBATLinkPlayerStats stats : m_mPlayerStats)
		{
			if (stats)
				stats.ResetPeriod();
		}
	}

	protected string BuildORBATPayload()
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

	protected void SendPayload(string payload, bool resetAfterSuccess = false)
	{
		if (payload.IsEmpty())
			return;

		if (!m_RestContext)
		{
			Print("[TFR_ORBATLink] No hay RestContext. Payload no enviado.", LogLevel.ERROR);
			return;
		}

		ref TFR_ORBATLinkRestCallback callback = new TFR_ORBATLinkRestCallback(this, resetAfterSuccess);
		m_aCallbacks.Insert(callback);

		int requestId = m_RestContext.POST(callback, m_Config.m_sRoute, payload);

		if (m_Config.m_bDebug)
		{
			Print("[TFR_ORBATLink] POST requestId=" + requestId.ToString(), LogLevel.NORMAL);
			Print("[TFR_ORBATLink] Payload=" + payload, LogLevel.NORMAL);
		}
	}

	void OnRestSuccess(TFR_ORBATLinkRestCallback callback)
	{
		if (!callback)
			return;

		int httpCode = callback.GetHttpCode();
		int restResult = callback.GetRestResult();
		string data = callback.GetData();
		bool resetAfterSuccess = callback.ShouldResetAfterSuccess();

		Print("[TFR_ORBATLink] REST SUCCESS. http=" + httpCode.ToString() + " rest=" + restResult.ToString(), LogLevel.NORMAL);

		if (m_Config && m_Config.m_bDebug && !data.IsEmpty())
			Print("[TFR_ORBATLink] Respuesta: " + data, LogLevel.NORMAL);

		m_aCallbacks.RemoveItem(callback);

		if (resetAfterSuccess)
			ResetAllPeriods();
	}

	void OnRestError(TFR_ORBATLinkRestCallback callback)
	{
		if (!callback)
			return;

		int httpCode = callback.GetHttpCode();
		int restResult = callback.GetRestResult();
		string data = callback.GetData();

		Print("[TFR_ORBATLink] REST ERROR. http=" + httpCode.ToString() + " rest=" + restResult.ToString(), LogLevel.ERROR);

		if (m_Config && m_Config.m_bDebug && !data.IsEmpty())
			Print("[TFR_ORBATLink] Respuesta error: " + data, LogLevel.ERROR);

		m_aCallbacks.RemoveItem(callback);
	}
}
