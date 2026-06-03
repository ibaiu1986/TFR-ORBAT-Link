//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkVehicleDamageHook.c
// Detecta vehiculos destruidos server-side.
//
// Criterio TFR:
// - No usa DamageHistory.
// - No usa ExtendedDamageManagerComponent.
// - Clasifica por componentes.
// - Atribuye primero a ultimo jugador con disparo reciente.
// - Si no hay disparo reciente, atribuye a ultimo jugador activo reciente.
// - Si no hay jugador reciente, solo loguea y no suma.

class TFR_ORBATLinkVehicleDamageHelper
{
	protected static int s_iLastCombatPlayerId;
	protected static int s_iLastCombatPlayerMs;

	protected static int s_iLastActivePlayerId;
	protected static int s_iLastActivePlayerMs;

	protected const int TFR_VEHICLE_COMBAT_WINDOW_MS = 60000;
	protected const int TFR_VEHICLE_ACTIVE_WINDOW_MS = 300000;

	static void RegisterRecentCombatPlayer(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		s_iLastCombatPlayerId = playerId;
		s_iLastCombatPlayerMs = System.GetTickCount();

		RegisterRecentActivePlayer(playerId);
	}

	static void RegisterRecentActivePlayer(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (playerId <= 0)
			return;

		s_iLastActivePlayerId = playerId;
		s_iLastActivePlayerMs = System.GetTickCount();
	}

	static int ResolveAttributionPlayer()
	{
		if (!Replication.IsServer())
			return 0;

		int now = System.GetTickCount();

		if (s_iLastCombatPlayerId > 0 && now - s_iLastCombatPlayerMs <= TFR_VEHICLE_COMBAT_WINDOW_MS)
			return s_iLastCombatPlayerId;

		if (s_iLastActivePlayerId > 0 && now - s_iLastActivePlayerMs <= TFR_VEHICLE_ACTIVE_WINDOW_MS)
			return s_iLastActivePlayerId;

		return 0;
	}

	static string ResolveVehicleCategoryByComponents(IEntity vehicle)
	{
		if (!vehicle)
			return "unknown";

		if (vehicle.FindComponent(SCR_HelicopterControllerComponent))
			return "air";

		if (vehicle.FindComponent(SCR_CarControllerComponent))
			return "light";

		if (vehicle.FindComponent(CarControllerComponent))
			return "light";

		return "unknown";
	}

	static string BuildVehicleComponentSummary(IEntity vehicle)
	{
		if (!vehicle)
			return "none";

		string summary = "";

		if (vehicle.FindComponent(SCR_HelicopterControllerComponent))
			summary += "SCR_HelicopterControllerComponent;";

		if (vehicle.FindComponent(SCR_CarControllerComponent))
			summary += "SCR_CarControllerComponent;";

		if (vehicle.FindComponent(CarControllerComponent))
			summary += "CarControllerComponent;";

		if (vehicle.FindComponent(VehicleControllerComponent))
			summary += "VehicleControllerComponent;";

		if (vehicle.FindComponent(BaseVehicleControllerComponent))
			summary += "BaseVehicleControllerComponent;";

		if (vehicle.FindComponent(SCR_VehicleDamageManagerComponent))
			summary += "SCR_VehicleDamageManagerComponent;";

		if (summary.IsEmpty())
			summary = "none";

		return summary;
	}
}

//------------------------------------------------------------------------------------------------

modded class SCR_VehicleDamageManagerComponent
{
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		if (!Replication.IsServer())
			return;

		if (isJIP)
			return;

		if (newState != EDamageState.DESTROYED)
			return;

		if (previousDamageState == EDamageState.DESTROYED)
			return;

		IEntity owner = GetOwner();

		if (!owner)
		{
			Print("[TFR_ORBATLink] Vehiculo destruido detectado, pero owner=null.", LogLevel.WARNING);
			return;
		}

		string category = TFR_ORBATLinkVehicleDamageHelper.ResolveVehicleCategoryByComponents(owner);
		string componentSummary = TFR_ORBATLinkVehicleDamageHelper.BuildVehicleComponentSummary(owner);

		string prefabName = "";
		EntityPrefabData prefabData = owner.GetPrefabData();

		if (prefabData)
			prefabName = prefabData.GetPrefabName();

		int playerId = TFR_ORBATLinkVehicleDamageHelper.ResolveAttributionPlayer();

		Print(string.Format("[TFR_ORBATLink] Vehiculo destruido detectado. attributedPlayerId=%1 category=%2 entityType=%3 components=%4 prefab=%5 entity=%6",
			playerId,
			category,
			owner.Type().ToString(),
			componentSummary,
			prefabName,
			owner.ToString()
		), LogLevel.NORMAL);

		if (playerId <= 0)
		{
			Print("[TFR_ORBATLink] Vehiculo destruido sin jugador reciente. No suma vehiculo.", LogLevel.WARNING);
			return;
		}

		TFR_ORBATLinkService.GetInstance().OnVehicleDestroyed(playerId, category);
	}
}
