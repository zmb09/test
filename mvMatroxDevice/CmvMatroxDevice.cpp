#include "StdAfx.h"
#include "CmvMatroxDevice.h"

CmvMatroxDevice::CmvMatroxDevice(LPUNKNOWN in_poUnknown, HRESULT*out_pHresult)
	: CMvUnknown( L"CmvMatroxDevice" , in_poUnknown)
{
	if(out_pHresult)
	*out_pHresult = MV_NOERROR;

	//if(!m_hClockEvent)
	//{
	//	m_hClockEvent = ::CreateEvent(NULL, false, false, NULL); 
	//}
	//m_bGetClockEvent = false;

}

CmvMatroxDevice::~CmvMatroxDevice()
{
	//	m_pJResourceManager = NULL;
	//
	//	m_vecOutputDeviceControl.clear();
	//	m_vecInputDeviceControl.clear();
}

//STDMETHODIMP CmvMatroxDevice::NonDelegatingQueryInterface(REFIID riid, void** ppv)
//{
//
//	//::MessageBoxA(NULL,NULL,NULL,MB_OK);
//
//	if (riid == IID_IMvSystemClockNotificationCallback)
//	{
//		return GetInterface(static_cast<IMvSystemClockNotificationCallback*>(this), ppv);
//	}
//	return CNsdDeviceBase::NonDelegatingQueryInterface(riid, ppv);
//}

HRESULT CmvMatroxDevice::Init( SMvResolutionInfo* in_psResolutionInfo)
{
	HRESULT hr = MV_NOERROR;

	m_sResolution = * in_psResolutionInfo;

	if (m_sResolution.ulComponentBitCount>10)
	{
		m_sResolution.ulComponentBitCount = 10;
	}

	ASSERT(m_pJSystemCardConfig);

	hr = _DestroyAllPreviousSystemTopology();
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );

	// Set asynch error callback
	hr = m_pJSystemCardConfig->GetErrorNotification(&m_pJErrorNotification);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );

	if (m_pJErrorNotification)
	{
		m_pJErrorCallback = new CErrorNotificationCallback;
		hr = m_pJErrorNotification->RegisterCallbackForNotification(m_pJErrorCallback);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
			"CmvMatroxDevice::_SetGenlockClock  error", true );
	}

	hr = _SetGenlockClock();
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );

	hr = _SetupGenlockResolution();
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );

	//hr = CreateClockThread();

	hr = _SetSystemClockCallback();
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );

	return hr;
}

HRESULT CmvMatroxDevice::AddSemaphore(HANDLE in_hSemaphore)
{
	HRESULT hr = MV_NOERROR;
	CMvAutoLock lock(&m_CritSecHandle);
	m_vecSemaphore.push_back(in_hSemaphore);

	return hr;
}
HRESULT CmvMatroxDevice::AddEvent(HANDLE in_hEvent)
{
	HRESULT hr = MV_NOERROR;
	CMvAutoLock lock(&m_CritSecHandle);
	m_vecEvent.push_back(in_hEvent);

	return hr;
}
//
//HRESULT CmvMatroxDevice::GetClock(INsdClock **out_ppINsdClock)
//{
//	HRESULT hr = MV_NOERROR;
//	if (out_ppINsdClock)
//	{
//		*out_ppINsdClock = this;
//	}
//	return hr;
//}

HRESULT __stdcall CmvMatroxDevice::GetCurrentTimes( uint64_t *out_pui64Time )
{
	HRESULT hr = MV_NOERROR;

	if (out_pui64Time)
	{
		*out_pui64Time = m_pJClock->GetTime();
	}

	return hr;
}

//
//HRESULT CmvMatroxDevice::GetEventHandle(HANDLE *out_phEvent)
//{
//	HRESULT hr = MV_NOERROR;
//
//	if (m_bGetClockEvent)
//	{
//		*out_phEvent = NULL;
//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,-1,
//			"CmvMatroxDevice::GetEventHandle  only can be call once!", true );
//	}
//
//	if (out_phEvent)
//	{
//		*out_phEvent = m_hClockEvent;
//	}
//	m_bGetClockEvent = true;
//
//	return hr;
//}

HRESULT CmvMatroxDevice::CreateChannel(CmvMatroxChannel** out_ppNsdOutputDeviceControl)
{
	HRESULT hr = MV_NOERROR;

	CmvMatroxChannel * MatroxChannel = new CmvMatroxChannel();
	MatroxChannel->m_pJSystemConfiguration = m_pJSystemConfiguration;
	MatroxChannel->m_pJSystemCardConfig = m_pJSystemCardConfig;
	MatroxChannel->m_pJClock = m_pJClock;
	MatroxChannel->m_sMvResolution = m_sResolution;
	//MatroxChannel->m_ichannelID = in_channelID;
	MatroxChannel->m_oCmvMatroxDevice = this;

	(*out_ppNsdOutputDeviceControl) = MatroxChannel;
	(*out_ppNsdOutputDeviceControl)->AddRef();

	return hr;
}

HRESULT CmvMatroxDevice::GetChannelByIndex(CmvMatroxChannel** out_ppNsdOutputDeviceControl,int *io_pchannelID)
{
	HRESULT hr = MV_NOERROR;

	CMvAutoLock lock(&m_CritSecOutputDeviceControl);
	//OutputDeviceControl_MAP_ITR itr = m_map_OutputDeviceControl.find(in_channelID);
	//if (itr != m_map_OutputDeviceControl.end())
	//{
	//	*out_ppNsdOutputDeviceControl = itr->second;
	//	return hr;
	//}

	if (io_pchannelID == 0 || *io_pchannelID == -1)
	{
		CmvMatroxChannel *OutputDeviceControlBase;
		CreateChannel(&OutputDeviceControlBase);
		
		//CNsdAJAChannel * AJAChannel = new CNsdAJAChannel();
		//MatroxChannel->m_pJSystemConfiguration = m_pJSystemConfiguration;
		//MatroxChannel->m_pJSystemCardConfig = m_pJSystemCardConfig;
		//MatroxChannel->m_pJClock = m_pJClock;
		//MatroxChannel->m_ichannelID = in_channelID;
		//MatroxChannel->m_oCNsdMatroxDevice = this;

		m_vector_pJOutputDeviceControl.push_back(OutputDeviceControlBase);
		OutputDeviceControlBase->m_nChannelID = m_vector_pJOutputDeviceControl.size() - 1;

		if (io_pchannelID != 0)
		{
			*io_pchannelID = m_vector_pJOutputDeviceControl.size() - 1;
		}

		(*out_ppNsdOutputDeviceControl) = OutputDeviceControlBase;
        //(*out_ppNsdOutputDeviceControl)->AddRef();
	}
	else 
	{
		if(*io_pchannelID<m_vector_pJOutputDeviceControl.size())
		{
			(*out_ppNsdOutputDeviceControl) = m_vector_pJOutputDeviceControl[*io_pchannelID];
			(*out_ppNsdOutputDeviceControl)->AddRef();
		}
		else
		{
			*out_ppNsdOutputDeviceControl = NULL;
		}
	}

	//m_map_OutputDeviceControl[in_channelID] = MatroxChannel;
	//*out_ppNsdOutputDeviceControl = MatroxChannel;
	//MatroxChannel->AddRef();
	return hr;
}

//HRESULT CmvMatroxDevice::GetChannelByIndex(INsdOutputDeviceControl** out_ppNsdOutputDeviceControl,int *io_pchannelID)
//{
//	HRESULT hr = MV_NOERROR;
//
//	CNsAutoLock lock(&m_CritSecOutputDeviceControl);
//	//OutputDeviceControl_MAP_ITR itr = m_map_OutputDeviceControl.find(in_channelID);
//	//if (itr != m_map_OutputDeviceControl.end())
//	//{
//	//	*out_ppNsdOutputDeviceControl = itr->second;
//	//	return hr;
//	//}
//
//	if (io_pchannelID == 0 || *io_pchannelID == -1)
//	{
//		CmvMatroxChannel * MatroxChannel = new CmvMatroxChannel();
//		MatroxChannel->m_pJSystemConfiguration = m_pJSystemConfiguration;
//		MatroxChannel->m_pJSystemCardConfig = m_pJSystemCardConfig;
//		MatroxChannel->m_pJClock = m_pJClock;
//		MatroxChannel->m_ichannelID = in_channelID;
//		MatroxChannel->m_oCmvMatroxDevice = this;
//
//		m_vector_pJOutputDeviceControl.push_back(MatroxChannel);
//
//		if (io_pchannelID != 0)
//		{
//			*io_pchannelID = m_vector_pJOutputDeviceControl.size() - 1;
//		}
//
//		*out_ppNsdOutputDeviceControl = m_vector_pJOutputDeviceControl[*io_pchannelID];
//		(*out_ppNsdOutputDeviceControl)->AddRef();
//	}
//	else 
//	{
//		if(*io_pchannelID<m_vector_pJOutputDeviceControl.size())
//		{
//			*out_ppNsdOutputDeviceControl = m_vector_pJOutputDeviceControl[*io_pchannelID];
//			(*out_ppNsdOutputDeviceControl)->AddRef();
//		}
//		else
//		{
//			*out_ppNsdOutputDeviceControl = NULL;
//		}
//	}
//
//
//
//	//m_map_OutputDeviceControl[in_channelID] = MatroxChannel;
//
//	//*out_ppNsdOutputDeviceControl = MatroxChannel;
//	//MatroxChannel->AddRef();
//
//	return hr;
//}

HRESULT CmvMatroxDevice::_DestroyAllPreviousSystemTopology()
{
	HRESULT hr;
	TMvSmartPtr<IMvTopologiesEnumerator>	pJTopologyEnumerator = NULL;
	TMvSmartPtr<IMvTopology>				pJTopology = NULL;

	//Clean up whatever is in the card.
	hr = m_pJSystemCardConfig->ClearDefaultsAndPermanentMemory();

	if SUCCEEDED(hr)
	{
		//WaitForSingleObject(m_ahDefaultStateSignaling[keDefaultStateClearedFlash], 5000);

		hr = m_pJSystemCardConfig->CreateTopologiesEnumerator(&pJTopologyEnumerator);
		if SUCCEEDED(hr)
		{
			hr = pJTopologyEnumerator->Next(&pJTopology);
			while (SUCCEEDED(hr))
			{
				hr = pJTopology->AcquireControl();
				if SUCCEEDED(hr)
				{
					hr = pJTopology->DestroyTopology();
					// Next topology
					if SUCCEEDED(hr)
					{
						pJTopology = NULL;
						hr = pJTopologyEnumerator->Next(&pJTopology);
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	if(hr == MV_E_END_OF_ENUM)
	{
		hr = MV_NOERROR;
	}
	return hr;
}

HRESULT CmvMatroxDevice::_SetGenlockClock()
{
	HRESULT hr;

	// Get the clock enumerator interface.
	TMvSmartPtr<IMvSystemClockEnumerator>	pJClockEnum;
	TMvSmartPtr<IMvSystemClock>				pJClock;

	hr = m_pJSystemCardConfig->CreateSystemClockEnumerator(&pJClockEnum);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );

	// Enumerate all clocks in the system.
	while (pJClockEnum->Next(&pJClock) == MV_NOERROR)
	{
		// Search for a genlock clock
		if (pJClock->GetClockType() == keMvSystemClockTypeGenlock)
		{
			TMvSmartQPtr<IMvSystemClockGenlock, &IID_IMvSystemClockGenlock> pQGenlock(pJClock, &hr);

			if SUCCEEDED(hr)
			{
				TMvSmartPtr<IMvCardConfiguration> pJTempCard;
				hr = pQGenlock->GetCardConfig(&pJTempCard);

				if (m_pJSystemCardConfig == pJTempCard)
				{
					m_pJClock = pJClock;		// This is the genlock clock corresponding to the used card.
					break;
				}
			}
		}

		pJClock = NULL;  // Next clock
	}

	return hr;
}

HRESULT CmvMatroxDevice::_SetupGenlockResolution()
{
	HRESULT hr;

	// Get genlock interface.         
	TMvSmartPtr<IMvGenlockConfiguration>	pJGenlockConfig;

	hr = m_pJSystemCardConfig->GetGenlockConfiguration(&pJGenlockConfig);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );
	// Get current genlock settings

	SMvGenlockSettings sCurrentGenlockSettings = { sizeof(sCurrentGenlockSettings) };
	hr = pJGenlockConfig->GetGenlockSettings(sCurrentGenlockSettings);

	SMvGenlockSettings sNewGenlockSettings;
	sNewGenlockSettings = sCurrentGenlockSettings;
	sNewGenlockSettings.eGenlockSource = keGenlockSourceInternal;

	SMvResolutionInfo	sResolution = m_sResolution;
	sResolution.ulWidth = 1920;
	sResolution.ulHeight = 1080;
	//sResolution.ulComponentBitCount = 10;
	sNewGenlockSettings.sResolution = sResolution;

	//sNewGenlockSettings.sResolution.eAspectRatio		= m_sResolution.eAspectRatio;
	//sNewGenlockSettings.sResolution.ulComponentBitCount = 8;//m_sResolution.ulComponentBitCount;
	//sNewGenlockSettings.sResolution.eAspectRatio		= keMvAspectRatio_4_3;

	//sNewGenlockSettings.sResolution.ulHeight	= 1080;
	//sNewGenlockSettings.sResolution.ulWidth		= 1920;
	//	sNewGenlockSettings.sResolution.eScanMode	= keMvScanModeFirstFieldTop;


	//if(sNewGenlockSettings.sResolution.eFrameRate = keMvFrameRate25)
	//{
	//	sNewGenlockSettings.sResolution.ulHeight	= 576;
	//	sNewGenlockSettings.sResolution.ulWidth		= 720;
	//	sNewGenlockSettings.sResolution.eScanMode	= keMvScanModeFirstFieldTop;
	//}
	//else
	//{
	//	sNewGenlockSettings.sResolution.ulHeight	= 486;
	//	sNewGenlockSettings.sResolution.ulWidth		= 720;
	//	sNewGenlockSettings.sResolution.eScanMode	= keMvScanModeSecondFieldTop;
	//}

	hr = pJGenlockConfig->SetGenlockSettings(sNewGenlockSettings);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetGenlockClock  error", true );
	return hr;
}

HRESULT CmvMatroxDevice::_SetSystemClockCallback()
{
	HRESULT		hr = MV_E_FAIL;
	uint64_t	ui64ClockStartTime;

	//hr = m_pJClock->GetTime();

	// Need frame event 
	SMvResolutionInfo sResolutionForFrame;

	sResolutionForFrame.size = sizeof(SMvResolutionInfo);
	sResolutionForFrame = m_sResolution;
	//sResolutionForFrame.eScanMode = keMvScanModeProgressive;

	ui64ClockStartTime = _GetNextClockTime();

	hr = m_pJClock->RegisterForPeriodicNotification(ui64ClockStartTime, m_sResolution, this);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDevice::_SetSystemClockCallback  error", true );

	return hr;
}

uint64_t CmvMatroxDevice::_GetNextClockTime()
{
	uint64_t    ui64NanoTime, ui64SampleTime;

	// Get the current time 
	ui64NanoTime = m_pJClock->GetTime();
	ui64NanoTime = MvGetNextSampleNanoTime(ui64NanoTime, m_sResolution);
	ui64SampleTime = MvGetSampleForNanoTime(ui64NanoTime, m_sResolution) + 2; 

	// Return nanotime
	return MvGetNanoTimeForSample(ui64SampleTime, m_sResolution);

}

void CmvMatroxDevice::NotifyClock()
{
	static int count = 0;
	count++;
	char temp[200];

	if(count<10)
	{
		sprintf(temp,"f_y,begin,testMvMatroxDevice,CmvMatroxDevice::NotifyClockcount=%d",count);
		OutputDebugStringA(temp);
	}
	
	Signal();

	if(count<10 || count%500 == 0 )
	{
		sprintf(temp,"f_y,end,testMvMatroxDevice,CmvMatroxDevice::NotifyClockcount=%d",count);
		OutputDebugStringA(temp);
	}
	
}

void CmvMatroxDevice::Signal()
{
	//SetEvent(m_hClockEvent);

	try
	{
		CMvAutoLock lock(&m_CritSecHandle);

		for (int i=0;i<m_vecEvent.size();i++)
		{
			SetEvent(m_vecEvent.at(i));
		}
		for (int i=0;i<m_vecSemaphore.size();i++)
		{
			ReleaseSemaphore(m_vecSemaphore.at(0),1,NULL);
		}
	}
	catch(...)
	{
		OutputDebugStringA("f_y,testMvMatroxDevice,CmvMatroxDevice::Signal,error");
	}
}

HRESULT CmvMatroxDevice::CreateClockThread()
{
	HRESULT hr = MV_NOERROR;
	try
	{
		m_oClockThread.Create(CmvMatroxDevice::ClockThreadRun, this);
	}
	catch (...)
	{
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncPlayBack,
			-1,	"CNsdOutputDeviceControlBase::SetClockInterface: m_oThread.Create Failed!", true);
	}

	return hr;
}

UINT CmvMatroxDevice::ClockThreadRun(void* pThis)
{
	CmvMatroxDevice* pThread = static_cast<CmvMatroxDevice*>(pThis);
	pThread->ClockThreadLoop();
	return 0;
}

HRESULT CmvMatroxDevice::ClockThreadLoop()
{
	HRESULT hr = MV_NOERROR;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	__int64 rtPeriod = MvGetNanoTimeForSample(1, m_sResolution);
	//__int64 rtPeriod = (REFERENCE_TIME)(CNsdTime::ResolutionUnit2Nano( 1 , m_NsResolutionInfo ));
	unsigned long ulTime = rtPeriod/10000;

	//GDQXNameSpace::Initialize();

	char temp[200];
	sprintf(temp,"f_y,CNsdDeviceBase::ClockThreadLoop,ulTime=%ums",ulTime);
	//GDQXNameSpace::GDQXWriteToFileA(temp);
	OutputDebugStringA(temp);

	while(1)
	{
		::Sleep(ulTime);
		Signal();
	}

	return hr;
}