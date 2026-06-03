class TFR_ORBATLinkRestCallback : RestCallback
{
	TFR_ORBATLinkService m_Service;
	bool m_bResetAfterSuccess;

	void TFR_ORBATLinkRestCallback(TFR_ORBATLinkService service, bool resetAfterSuccess = false)
	{
		m_Service = service;
		m_bResetAfterSuccess = resetAfterSuccess;

		SetOnSuccess(OnTFRSuccess);
		SetOnError(OnTFRError);
	}

	bool ShouldResetAfterSuccess()
	{
		return m_bResetAfterSuccess;
	}

	void OnTFRSuccess(RestCallback callback)
	{
		if (!m_Service)
			return;

		TFR_ORBATLinkRestCallback tfrCallback = TFR_ORBATLinkRestCallback.Cast(callback);

		if (!tfrCallback)
			return;

		m_Service.OnRestSuccess(tfrCallback);
	}

	void OnTFRError(RestCallback callback)
	{
		if (!m_Service)
			return;

		TFR_ORBATLinkRestCallback tfrCallback = TFR_ORBATLinkRestCallback.Cast(callback);

		if (!tfrCallback)
			return;

		m_Service.OnRestError(tfrCallback);
	}
}
