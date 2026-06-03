class TFR_ORBATLinkRestCallback : RestCallback
{
	TFR_ORBATLinkService m_Service;

	void TFR_ORBATLinkRestCallback(TFR_ORBATLinkService service)
	{
		m_Service = service;

		SetOnSuccess(OnTFRSuccess);
		SetOnError(OnTFRError);
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
