//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkMedicalHook.c
// Hooks medicos seguros para ORBATLink.
//
// Criterio TFR:
// - No toca CanBePerformed.
// - No toca PerformAction.
// - No bloquea acciones medicas.
// - Cuenta consumibles cuando el efecto ya se ha aplicado.
// - Torniquete NO se cuenta desde SCR_ConsumableTourniquet directamente.
// - Torniquete se cuenta desde SCR_TourniquetStorageComponent.OnAddedToSlot,
//   que confirma que el torniquete ha quedado colocado.
// - Quitar torniquete NO cuenta.

modded class SCR_ConsumableEffectHealthItems
{
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		super.ApplyEffect(target, user, item, animParams);

		if (!Replication.IsServer())
			return;

		if (!user)
			return;

		if (!item)
			return;

		SCR_ConsumableItemComponent consumableItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));

		if (!consumableItem)
			return;

		SCR_EConsumableType typeId = consumableItem.GetConsumableType();
		string medicalCode = TFR_ORBATLink_ResolveMedicalCode(typeId);

		if (medicalCode.IsEmpty())
			return;

		// El torniquete se cuenta por SCR_TourniquetStorageComponent.OnAddedToSlot.
		if (medicalCode == "TOURNIQUET")
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return;

		int playerId = playerManager.GetPlayerIdFromControlledEntity(user);

		if (playerId <= 0)
			return;

		TFR_ORBATLinkService.GetInstance().OnMedicalConsumableUsedByCode(
			playerId,
			medicalCode,
			typeId.ToString(),
			"SCR_ConsumableEffectHealthItems"
		);
	}

	protected string TFR_ORBATLink_ResolveMedicalCode(SCR_EConsumableType typeId)
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
}

//------------------------------------------------------------------------------------------------
// Hook seguro de torniquete.
// Cuenta cuando el item entra realmente en el slot de torniquete del personaje.

modded class SCR_TourniquetStorageComponent
{
	protected static ref map<string, int> s_mTFR_ORBATLinkPendingTourniquets;

	override void AddTourniquetToSlot(IEntity target, ECharacterHitZoneGroup eHitZoneGroup, IEntity tourniquet)
	{
		if (Replication.IsServer() && tourniquet)
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();

			if (playerManager)
			{
				int applierPlayerId = playerManager.GetPlayerIdFromControlledEntity(GetOwner());

				if (applierPlayerId > 0)
				{
					if (!s_mTFR_ORBATLinkPendingTourniquets)
						s_mTFR_ORBATLinkPendingTourniquets = new map<string, int>();

					string key = TFR_ORBATLink_GetTourniquetKey(tourniquet);

					if (s_mTFR_ORBATLinkPendingTourniquets.Contains(key))
						s_mTFR_ORBATLinkPendingTourniquets.Remove(key);

					s_mTFR_ORBATLinkPendingTourniquets.Insert(key, applierPlayerId);
				}
			}
		}

		super.AddTourniquetToSlot(target, eHitZoneGroup, tourniquet);
	}

	override void OnAddedToSlot(IEntity item, int slotID)
	{
		super.OnAddedToSlot(item, slotID);

		if (!Replication.IsServer())
			return;

		if (!item)
			return;

		int playerId = TFR_ORBATLink_ResolveTourniquetApplier(item);

		if (playerId <= 0)
			return;

		TFR_ORBATLinkService.GetInstance().OnMedicalConsumableUsedByCode(
			playerId,
			"TOURNIQUET",
			"SCR_EConsumableType.TOURNIQUET",
			"SCR_TourniquetStorageComponent.OnAddedToSlot"
		);

		Print(string.Format("[TFR_ORBATLink] Torniquete colocado registrado. playerId=%1 target=%2 item=%3 slotID=%4",
			playerId,
			GetOwner(),
			item,
			slotID
		), LogLevel.NORMAL);
	}

	protected string TFR_ORBATLink_GetTourniquetKey(IEntity item)
	{
		if (!item)
			return "";

		return item.ToString();
	}

	protected int TFR_ORBATLink_ResolveTourniquetApplier(IEntity item)
	{
		if (!item)
			return 0;

		string key = TFR_ORBATLink_GetTourniquetKey(item);

		if (s_mTFR_ORBATLinkPendingTourniquets && s_mTFR_ORBATLinkPendingTourniquets.Contains(key))
		{
			int playerId = s_mTFR_ORBATLinkPendingTourniquets.Get(key);
			s_mTFR_ORBATLinkPendingTourniquets.Remove(key);

			if (playerId > 0)
				return playerId;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return 0;

		return playerManager.GetPlayerIdFromControlledEntity(GetOwner());
	}
}
