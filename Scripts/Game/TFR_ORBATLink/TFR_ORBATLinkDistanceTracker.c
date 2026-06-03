//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkDistanceTracker.c
// Mide distancia recorrida por jugador en servidor.
// No usa EOnFrame. Usa CallLater cada pocos segundos.
// Separa distancia a pie y distancia en vehiculo usando ChimeraCharacter.IsInVehicle().

class TFR_ORBATLinkDistanceTracker
{
	protected static ref TFR_ORBATLinkDistanceTracker s_Instance;

	protected ref map<int, vector> m_mLastPositions;
	protected ref array<int> m_aTrackedPlayers;

	protected bool m_bRunning;

	protected const int TFR_DISTANCE_TICK_MS = 5000;
	protected const float TFR_MIN_DISTANCE_M = 1.0;
	protected const float TFR_MAX_DISTANCE_PER_TICK_M = 500.0;

	static TFR_ORBATLinkDistanceTracker GetInstance()
	{
		if (!s_Instance)
			s_Instance = new TFR_ORBATLinkDistanceTracker();

		return s_Instance;
	}

	void TFR_ORBATLinkDistanceTracker()
	{
		m_mLastPositions = new map<int, vector>();
		m_aTrackedPlayers = new array<int>();
		m_bRunning = false;
	}

	void Start()
	{
		if (!Replication.IsServer())
			return;

		if (m_bRunning)
			return;

		m_bRunning = true;

		GetGame().GetCallqueue().CallLater(Tick, TFR_DISTANCE_TICK_MS, true);

		Print("[TFR_ORBATLink] Distance tracker iniciado.", LogLevel.NORMAL);
	}

	void Stop()
	{
		if (!m_bRunning)
			return;

		m_bRunning = false;

		GetGame().GetCallqueue().Remove(Tick);

		if (m_mLastPositions)
			m_mLastPositions.Clear();

		if (m_aTrackedPlayers)
			m_aTrackedPlayers.Clear();

		Print("[TFR_ORBATLink] Distance tracker detenido.", LogLevel.NORMAL);
	}

	void RegisterPlayer(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		Start();

		if (!m_aTrackedPlayers.Contains(playerId))
			m_aTrackedPlayers.Insert(playerId);

		IEntity controlledEntity = GetControlledEntity(playerId);

		if (!controlledEntity)
			return;

		vector pos = controlledEntity.GetOrigin();

		if (m_mLastPositions.Contains(playerId))
			m_mLastPositions.Remove(playerId);

		m_mLastPositions.Insert(playerId, pos);

		Print("[TFR_ORBATLink] Distance tracker registrado playerId=" + playerId.ToString(), LogLevel.NORMAL);
	}

	void UnregisterPlayer(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		m_aTrackedPlayers.RemoveItem(playerId);

		if (m_mLastPositions.Contains(playerId))
			m_mLastPositions.Remove(playerId);

		Print("[TFR_ORBATLink] Distance tracker eliminado playerId=" + playerId.ToString(), LogLevel.NORMAL);
	}

	protected void Tick()
	{
		if (!Replication.IsServer())
			return;

		if (!m_bRunning)
			return;

		if (!m_aTrackedPlayers)
			return;

		if (m_aTrackedPlayers.IsEmpty())
			return;

		foreach (int playerId : m_aTrackedPlayers)
		{
			ProcessPlayer(playerId);
		}
	}

	protected void ProcessPlayer(int playerId)
	{
		if (playerId <= 0)
			return;

		IEntity controlledEntity = GetControlledEntity(playerId);

		if (!controlledEntity)
			return;

		vector currentPos = controlledEntity.GetOrigin();

		if (!m_mLastPositions.Contains(playerId))
		{
			m_mLastPositions.Insert(playerId, currentPos);
			return;
		}

		vector lastPos = m_mLastPositions.Get(playerId);

		float distance = vector.Distance(lastPos, currentPos);

		if (m_mLastPositions.Contains(playerId))
			m_mLastPositions.Remove(playerId);

		m_mLastPositions.Insert(playerId, currentPos);

		if (distance < TFR_MIN_DISTANCE_M)
			return;

		// Evita sumar teleports, respawns o saltos raros de streaming.
		if (distance > TFR_MAX_DISTANCE_PER_TICK_M)
			return;

		int meters = Math.Round(distance);

		if (meters <= 0)
			return;

		if (IsPlayerInVehicle(controlledEntity))
			TFR_ORBATLinkService.GetInstance().OnDistanceInVehicle(playerId, meters);
		else
			TFR_ORBATLinkService.GetInstance().OnDistanceWalked(playerId, meters);
	}

	protected IEntity GetControlledEntity(int playerId)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return null;

		return playerManager.GetPlayerControlledEntity(playerId);
	}

	protected bool IsPlayerInVehicle(IEntity controlledEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);

		if (!character)
			return false;

		return character.IsInVehicle();
	}
}
