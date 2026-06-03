modded class SCR_BaseGameMode
{
	protected ref map<int, IEntity> m_mTFR_ORBATLink_ShotTrackedPlayers;

	override void OnGameStart()
	{
		super.OnGameStart();

		if (!Replication.IsServer())
			return;

		TFR_ORBATLink_EnsureShotTracker();

		Print("[TFR_ORBATLink] Hook OnGameStart", LogLevel.NORMAL);

		TFR_ORBATLinkService.GetInstance().Init();
		TFR_ORBATLinkDistanceTracker.GetInstance().Start();
	}

	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		if (!Replication.IsServer())
			return;

		Print("[TFR_ORBATLink] Hook OnPlayerRegistered playerId=" + playerId.ToString(), LogLevel.NORMAL);

		TFR_ORBATLinkService.GetInstance().OnPlayerRegistered(playerId);
	}

	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		if (!Replication.IsServer())
			return;

		Print("[TFR_ORBATLink] Hook OnPlayerSpawned playerId=" + playerId.ToString(), LogLevel.NORMAL);

		TFR_ORBATLinkService.GetInstance().OnPlayerRegistered(playerId);

		TFR_ORBATLink_RegisterShotEvents(playerId, controlledEntity);
		TFR_ORBATLinkDistanceTracker.GetInstance().RegisterPlayer(playerId);
	}

	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		if (Replication.IsServer())
		{
			TFR_ORBATLink_UnregisterShotEvents(playerId);
			TFR_ORBATLinkDistanceTracker.GetInstance().UnregisterPlayer(playerId);

			Print("[TFR_ORBATLink] Hook OnPlayerDisconnected playerId=" + playerId.ToString(), LogLevel.NORMAL);
			TFR_ORBATLinkService.GetInstance().OnPlayerDisconnected(playerId);
		}

		super.OnPlayerDisconnected(playerId, cause, timeout);
	}

	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);

		if (!Replication.IsServer())
			return;

		Print("[TFR_ORBATLink] Hook OnPlayerKilled victimPlayerId=" + playerId.ToString(), LogLevel.NORMAL);

		TFR_ORBATLinkService.GetInstance().OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
	}

	override void OnGameModeEnd(SCR_GameModeEndData endData)
	{
		if (Replication.IsServer())
		{
			Print("[TFR_ORBATLink] Hook OnGameModeEnd", LogLevel.NORMAL);

			TFR_ORBATLinkService.GetInstance().SendAll();
			TFR_ORBATLinkDistanceTracker.GetInstance().Stop();
		}

		super.OnGameModeEnd(endData);
	}

	protected void TFR_ORBATLink_EnsureShotTracker()
	{
		if (!m_mTFR_ORBATLink_ShotTrackedPlayers)
			m_mTFR_ORBATLink_ShotTrackedPlayers = new map<int, IEntity>();
	}

	protected void TFR_ORBATLink_RegisterShotEvents(int playerId, IEntity controlledEntity)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		if (!controlledEntity)
			return;

		TFR_ORBATLink_EnsureShotTracker();
		TFR_ORBATLink_UnregisterShotEvents(playerId);

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(controlledEntity.FindComponent(EventHandlerManagerComponent));

		if (!eventHandlerManager)
		{
			Print("[TFR_ORBATLink] No hay EventHandlerManagerComponent para registrar disparos playerId=" + playerId.ToString(), LogLevel.WARNING);
			return;
		}

		eventHandlerManager.RemoveScriptHandler("OnProjectileShot", this, TFR_ORBATLink_OnProjectileShot);
		eventHandlerManager.RegisterScriptHandler("OnProjectileShot", this, TFR_ORBATLink_OnProjectileShot);

		if (m_mTFR_ORBATLink_ShotTrackedPlayers.Contains(playerId))
			m_mTFR_ORBATLink_ShotTrackedPlayers.Remove(playerId);

		m_mTFR_ORBATLink_ShotTrackedPlayers.Insert(playerId, controlledEntity);

		Print("[TFR_ORBATLink] Shot hook registrado playerId=" + playerId.ToString(), LogLevel.NORMAL);
	}

	protected void TFR_ORBATLink_UnregisterShotEvents(int playerId)
	{
		if (!Replication.IsServer())
			return;

		TFR_ORBATLink_EnsureShotTracker();

		if (playerId <= 0)
			return;

		if (!m_mTFR_ORBATLink_ShotTrackedPlayers.Contains(playerId))
			return;

		IEntity controlledEntity = m_mTFR_ORBATLink_ShotTrackedPlayers.Get(playerId);

		if (controlledEntity)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(controlledEntity.FindComponent(EventHandlerManagerComponent));

			if (eventHandlerManager)
				eventHandlerManager.RemoveScriptHandler("OnProjectileShot", this, TFR_ORBATLink_OnProjectileShot);
		}

		m_mTFR_ORBATLink_ShotTrackedPlayers.Remove(playerId);
	}

	protected void TFR_ORBATLink_OnProjectileShot(int playerId, BaseWeaponComponent weapon, IEntity entity)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		TFR_ORBATLinkVehicleDamageHelper.RegisterRecentCombatPlayer(playerId);
		TFR_ORBATLinkService.GetInstance().OnShotFired(playerId);
	}
}
