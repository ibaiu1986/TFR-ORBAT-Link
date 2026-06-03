//------------------------------------------------------------------------------------------------
// TFR_ORBATLinkDamageHitHook.c
// Cuenta impactos reales para precision.
//
// Criterio TFR:
// - Server-side.
// - Solo personajes.
// - Solo EDamageType.KINETIC.
// - No cuenta curacion, sangrado, explosiones, fuego, colisiones ni melee.
// - No cuenta disparos contra uno mismo.
// - Deduplicacion en Service para evitar multiples hitzones del mismo impacto.

modded class SCR_DamageManagerComponent
{
	override protected void OnDamage(notnull BaseDamageContext damageContext)
	{
		super.OnDamage(damageContext);

		if (!Replication.IsServer())
			return;

		if (damageContext.damageValue <= 0)
			return;

		if (damageContext.damageType != EDamageType.KINETIC)
			return;

		IEntity owner = GetOwner();

		if (!owner)
			return;

		ChimeraCharacter victimCharacter = ChimeraCharacter.Cast(owner);

		if (!victimCharacter)
			return;

		Instigator instigator = GetInstigator();

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return;

		int shooterPlayerId = instigator.GetInstigatorPlayerID();

		if (shooterPlayerId <= 0)
		{
			IEntity instigatorEntity = instigator.GetInstigatorEntity();

			if (instigatorEntity)
				shooterPlayerId = playerManager.GetPlayerIdFromControlledEntity(instigatorEntity);
		}

		if (shooterPlayerId <= 0)
			return;

		int victimPlayerId = playerManager.GetPlayerIdFromControlledEntity(owner);

		if (victimPlayerId > 0 && victimPlayerId == shooterPlayerId)
			return;

		TFR_ORBATLinkService.GetInstance().OnShotHit(shooterPlayerId, owner);
	}
}
