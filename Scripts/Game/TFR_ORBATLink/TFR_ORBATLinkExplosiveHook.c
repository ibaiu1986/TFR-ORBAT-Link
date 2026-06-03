//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkExplosiveHook.c
// Cuenta detonaciones reales de explosivos colocados mediante SCR_ExplosiveTriggerComponent.
//
// Criterio TFR:
// - Cuenta la detonacion cuando se ejecuta el trigger de la carga.
// - Da igual si viene de detonador remoto/clacker, temporizador u otro trigger.
// - No cuenta RPG, granadas, disparos explosivos ni acciones de armar/configurar.
// - No toca kills, deaths, distancia, vehiculos, medicos, disparos ni REST.
//
// Nota:
// - El usuario se captura desde SCR_BaseTriggerComponent.SetUser(...).
// - El detonador vanilla llama a trigger.SetUser(m_User) antes de trigger.UseTrigger().

modded class SCR_ExplosiveTriggerComponent
{
	protected IEntity m_TFR_ORBATLink_TriggerUser;
	protected bool m_bTFR_ORBATLink_DetonationCounted;

	override void SetUser(notnull IEntity user)
	{
		super.SetUser(user);

		m_TFR_ORBATLink_TriggerUser = user;
	}

	override void UseTrigger()
	{
		IEntity triggerUser = m_TFR_ORBATLink_TriggerUser;
		bool alreadyCounted = m_bTFR_ORBATLink_DetonationCounted;
		bool canTriggerHere = TFR_ORBATLink_CanTriggerOnThisMachine();

		super.UseTrigger();

		if (!Replication.IsServer())
			return;

		if (!canTriggerHere)
			return;

		if (alreadyCounted)
			return;

		if (!triggerUser)
		{
			Print("[TFR_ORBATLink] Detonacion explosivo colocado detectada sin usuario asignado. No suma.", LogLevel.WARNING);
			return;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return;

		int playerId = playerManager.GetPlayerIdFromControlledEntity(triggerUser);

		if (playerId <= 0)
		{
			Print("[TFR_ORBATLink] Detonacion explosivo colocado sin playerId valido. No suma.", LogLevel.WARNING);
			return;
		}

		m_bTFR_ORBATLink_DetonationCounted = true;

		TFR_ORBATLinkService.GetInstance().OnPlacedExplosiveDetonated(playerId);

		Print(string.Format("[TFR_ORBATLink] Detonacion explosivo colocado registrada. playerId=%1 trigger=%2 owner=%3",
			playerId,
			this.ToString(),
			GetOwner().ToString()
		), LogLevel.NORMAL);
	}

	protected bool TFR_ORBATLink_CanTriggerOnThisMachine()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return false;

		RplComponent rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));

		if (!rplComp)
			return false;

		if (rplComp.IsProxy() && !rplComp.IsOwnerProxy())
			return false;

		return true;
	}
}
