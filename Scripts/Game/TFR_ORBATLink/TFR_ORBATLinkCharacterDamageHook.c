modded class SCR_CharacterDamageManagerComponent
{
	protected int m_iTFR_ORBATLink_LastDamagePlayerId;

	override void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		super.OnDamageEffectAdded(dmgEffect);

		if (!Replication.IsServer())
			return;

		Instigator instigator = dmgEffect.GetInstigator();

		if (!instigator)
			return;

		int playerId = instigator.GetInstigatorPlayerID();

		if (playerId <= 0)
			return;

		m_iTFR_ORBATLink_LastDamagePlayerId = playerId;
	}

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

		if (m_iTFR_ORBATLink_LastDamagePlayerId <= 0)
			return;

		TFR_ORBATLinkService.GetInstance().OnAIKilled(m_iTFR_ORBATLink_LastDamagePlayerId);
	}
}
