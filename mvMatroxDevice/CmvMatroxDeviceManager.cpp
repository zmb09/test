#include "StdAfx.h"

#include "CmvMatroxDeviceManager.h"

CmvMatroxDeviceManager::CmvMatroxDeviceManager(LPUNKNOWN in_poUnknown, HRESULT* out_pHresult)
	: CMvUnknown( L"CmvMatroxDeviceManager" , in_poUnknown)
{
	if (out_pHresult)
	{
		*out_pHresult = MV_NOERROR;
	}
	
}

CmvMatroxDeviceManager::~CmvMatroxDeviceManager()
{

}

CMvUnknown* __stdcall CmvMatroxDeviceManager::CreateInstance(IUnknown * in_pUnkOuter, HRESULT *io_phr)
{
	//::MessageBoxA(NULL,NULL,NULL,MB_OK);
	*io_phr = MV_NOERROR;
	CMvUnknown *pUnk = new CmvMatroxDeviceManager( in_pUnkOuter, io_phr );
	return pUnk;
}


void __stdcall CmvMatroxDeviceManager::DllInit(bool in_bIsLoading, const CLSID *in_psClsID)
{
	UNREFERENCED_PARAMETER(in_psClsID);
	if(!in_bIsLoading)
	{
	}
}

HRESULT __stdcall CmvMatroxDeviceManager::Init( SMvResolutionInfo* in_psResolutionInfo)
{
	HRESULT hr = MV_NOERROR;

	::CoInitialize(NULL);

	// 1.Initialize system configuration
	hr = CoCreateInstance(CLSID_MvSystemConfiguration,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IMvSystemConfiguration,
		(void**)&m_pJSystemConfiguration);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );

		// Get the cards enumerator
		TMvSmartPtr<IMvSystemConfigurationCardEnumerator> pJEnum;
		hr = m_pJSystemConfiguration->CreateCardEnumerator(&pJEnum);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );

		// Enumerate all cards
		if SUCCEEDED(hr)
		{	
			TMvSmartPtr<IMvCardConfiguration>		pJSystemCardConfig;
			hr = pJEnum->Next(&pJSystemCardConfig);

			if SUCCEEDED(hr)	// Get card information
			{
				//m_sCardInfo.sHwInfo.size = sizeof(m_sCardInfo.sHwInfo);
				//m_sCardInfo.sHwInfo.sConnectorsDescription.size = sizeof(m_sCardInfo.sHwInfo.sConnectorsDescription);

				//hr = m_pJSystemCardConfig->GetCardInformation(&m_sCardInfo.sHwInfo, 0);

				CmvMatroxDevice *MatroxDevice = new CmvMatroxDevice();
				m_vector_Device.push_back(MatroxDevice);
				MatroxDevice->m_pJSystemCardConfig = pJSystemCardConfig;
				MatroxDevice->m_pJSystemConfiguration = m_pJSystemConfiguration;
				hr = MatroxDevice->Init(in_psResolutionInfo);
				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
					"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );

				if SUCCEEDED(hr)                  
				{
					//hr = _FillConnectors();

					if SUCCEEDED(hr)
					{
						//hr = _ValidateSystemInputOutput();
					}
				}
			}
		}

	return hr;
}

HRESULT __stdcall CmvMatroxDeviceManager::GetDeviceCount(int *out_CardCount)
{
	HRESULT hr = MV_NOERROR;

	*out_CardCount = m_vector_Device.size();

	return hr;
}

HRESULT __stdcall CmvMatroxDeviceManager::GetOutputDeviceByIndex
	(
	CmvMatroxChannel** out_ppNsdOutputDeviceControl,
	int *io_pchannelID,
	int in_cardID
	)
{
	HRESULT hr = MV_NOERROR;

	if (out_ppNsdOutputDeviceControl)
	{
		hr = m_vector_Device[in_cardID]->GetChannelByIndex(out_ppNsdOutputDeviceControl,io_pchannelID);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );
	}

	return hr;
}


HRESULT __stdcall CmvMatroxDeviceManager::AddSemaphoreByIndex(HANDLE in_hSemaphore,int in_cardID)
{
	HRESULT hr = MV_NOERROR;

	if (in_hSemaphore)
	{
		hr = m_vector_Device[in_cardID]->AddSemaphore(in_hSemaphore);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );
	}

	return hr;
}
HRESULT __stdcall CmvMatroxDeviceManager::AddEventByIndex(HANDLE in_hEvent,int in_cardID)
{
	HRESULT hr = MV_NOERROR;

	if (in_hEvent)
	{
		hr = m_vector_Device[in_cardID]->AddEvent(in_hEvent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );
	}

	return hr;
}

//HRESULT __stdcall CmvMatroxDeviceManager::GetClockByIndex(INsdClock **out_ppINsdClock,int in_cardID)
//{
//	HRESULT hr = MV_NOERROR;
//
//	if (out_ppINsdClock)
//	{
//		hr = m_vector_Device[in_cardID]->GetClock(out_ppINsdClock);
//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
//			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );
//	}
//
//	return hr;
//}

//HRESULT __stdcall CmvMatroxDeviceManager::GetClockEventByIndex(HANDLE *out_hClockEvent,int in_cardID)
//{
//	HRESULT hr = MV_NOERROR;
//
//	if (out_hClockEvent)
//	{
//		hr = m_vector_Device[in_cardID]->GetClockEvent(out_hClockEvent);
//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
//			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration error", true );
//	}
//
//	return hr;
//}

//HRESULT __stdcall CmvMatroxDeviceManager::GetDeviceInfoByIndex
//	(
//	SNsDeviceInfoEx* out_sDeviceInfo,
//	int in_cardID
//	)
//{
//	HRESULT hr = MV_NOERROR;
//
//	if (out_sDeviceInfo)
//	{
//		out_sDeviceInfo->size = sizeof(SNsDeviceInfoEx);
//		out_sDeviceInfo->eDeviceType = keNsDeviceTypeMatrox;
//	}
//
//	return hr;
//}