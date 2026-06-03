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
			medicalCode = TFR_ORBATLink_ResolveMedicalCodeFromItem(item, typeId);

		if (medicalCode.IsEmpty())
		{
			Print(string.Format("[TFR_ORBATLink] Consumible medico aplicado pero no clasificado. type=%1 item=%2 prefab=%3",
				typeId.ToString(),
				item,
				TFR_ORBATLink_GetItemPrefabName(item)
			), LogLevel.WARNING);
			return;
		}

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

		Print(string.Format("[TFR_ORBATLink] Consumible medico registrado. playerId=%1 code=%2 type=%3 item=%4 prefab=%5",
			playerId,
			medicalCode,
			typeId.ToString(),
			item,
			TFR_ORBATLink_GetItemPrefabName(item)
		), LogLevel.NORMAL);
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

	protected string TFR_ORBATLink_ResolveMedicalCodeFromItem(IEntity item, SCR_EConsumableType typeId)
	{
		string typeName = typeId.ToString();
		string prefabName = TFR_ORBATLink_GetItemPrefabName(item);
		string itemName = "";

		if (item)
			itemName = item.ToString();

		if (TFR_ORBATLink_TextLooksLikeSaline(typeName))
			return "SALINE";

		if (TFR_ORBATLink_TextLooksLikeSaline(prefabName))
			return "SALINE";

		if (TFR_ORBATLink_TextLooksLikeSaline(itemName))
			return "SALINE";

		if (typeName.Contains("BANDAGE") || prefabName.Contains("Bandage") || prefabName.Contains("bandage"))
			return "BANDAGE";

		if (typeName.Contains("TOURNIQUET") || prefabName.Contains("Tourniquet") || prefabName.Contains("tourniquet"))
			return "TOURNIQUET";

		if (typeName.Contains("MORPHINE") || prefabName.Contains("Morphine") || prefabName.Contains("morphine"))
			return "MORPHINE";

		if (typeName.Contains("EPINEPHRINE") || prefabName.Contains("Epinephrine") || prefabName.Contains("epinephrine") || prefabName.Contains("EpiPen"))
			return "EPINEPHRINE";

		return "";
	}

	protected bool TFR_ORBATLink_TextLooksLikeSaline(string text)
	{
		if (text.IsEmpty())
			return false;

		if (text.Contains("SALINE"))
			return true;

		if (text.Contains("Saline"))
			return true;

		if (text.Contains("saline"))
			return true;

		if (text.Contains("SODIUM"))
			return true;

		if (text.Contains("Sodium"))
			return true;

		if (text.Contains("sodium"))
			return true;

		if (text.Contains("NACL"))
			return true;

		if (text.Contains("NaCl"))
			return true;

		if (text.Contains("nacl"))
			return true;

		if (text.Contains("IVBag"))
			return true;

		if (text.Contains("IV_Bag"))
			return true;

		if (text.Contains("Infusion"))
			return true;

		if (text.Contains("infusion"))
			return true;

		return false;
	}

	protected string TFR_ORBATLink_GetItemPrefabName(IEntity item)
	{
		if (!item)
			return "";

		EntityPrefabData prefabData = item.GetPrefabData();

		if (!prefabData)
			return "";

		return prefabData.GetPrefabName();
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
