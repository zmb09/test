#include "StdAfx.h"
#include "CmvMatroxChannel.h"
//#include "NsHighResTimer.h"

// {B87A1A21-9466-4DCE-8010-B502257E407B}
static const GUID CLSID_mvMatroxDevice = 
{ 0xb87a1a21, 0x9466, 0x4dce, { 0x80, 0x10, 0xb5, 0x2, 0x25, 0x7e, 0x40, 0x7b } };


CmvMatroxChannel::CmvMatroxChannel(LPUNKNOWN in_poUnknown, HRESULT* out_pHresult)
	: CMvUnknown( L"CmvMatroxChannel" , in_poUnknown)/*
	, m_pListpJNsOutputSurface(FALSE, "m_pListpJNsOutputSurface")
	, m_pListpJNsOutputAudioSamples(FALSE, "m_pListpJNsOutputAudioSamples")*/
{
	//*out_pHresult = MV_NOERROR;
	m_bClearBuffer = false;
	//m_pJRefClock = NULL;
	//m_pJNsdGraphStateManager = NULL;
	m_pJAudioVideoSurfaceAsynchErroCallback = NULL;
	m_eAudioChannelType = keChannelTypeStereo;
	m_bExist = false;
	m_hsemaphoreClock = CreateSemaphore(NULL, 0, 0xFF, NULL);
	if(m_hsemaphoreClock == NULL)
	{
		mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture, 
			-1, "m_hsemaphoreClock = CreateSemaphore(NULL, 0, 0xFF, NULL); error",true);
	}

}

CmvMatroxChannel::~CmvMatroxChannel()
{
	m_bExist = true;
}

//STDMETHODIMP CmvMatroxChannel::NonDelegatingQueryInterface(REFIID riid, void** ppv)
//{
//	if (riid == IID_INsdOutputDeviceControl)
//	{
//		return GetInterface(static_cast<INsdOutputDeviceControl*>(this), ppv);
//	}
//	return CmvMatroxChannel::NonDelegatingQueryInterface(riid, ppv);
//}

//HRESULT __stdcall CmvMatroxChannel::Init()
//{
//	HRESULT hr = MV_NOERROR;
//
//	return hr;
//}

//HRESULT __stdcall CmvMatroxChannel::SetGraphStateManager
//	(
//	INsGraphStateManager* in_pINsdGraphStateManager
//	)
//{
//	HRESULT hr = MV_NOERROR;
//
//	TMvSmartPtr<INsGraphStateManager>  pJNsdGraphStateManager = NULL;
//	hr = in_pINsdGraphStateManager->QueryInterface(IID_INsGraphStateManager, (void**)&pJNsdGraphStateManager);
//	if (SUCCEEDED(hr))
//	{
//		m_pJNsdGraphStateManager = in_pINsdGraphStateManager;
//
//		hr = m_pJNsdGraphStateManager->GetClock(&m_pJRefClock);
//		mv_SAFE_REPORT_ERROR_RETURN( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture, 
//			hr, "m_hsemaphoreClock = CreateSemaphore(NULL, 0, 0xFF, NULL); error",true);
//	}
//	else 
//	{
//		TMvSmartPtr<IReferenceClock> in_pJRefClock = (IReferenceClock*)in_pINsdGraphStateManager;
//		TMvSmartPtr<IReferenceClock> pJRefClock;
//		hr = in_pJRefClock->QueryInterface(IID_IReferenceClock, (void**)&pJRefClock);
//		if (SUCCEEDED(hr))
//		{
//			m_pJRefClock = pJRefClock;
//		}
//		else
//		{
//			mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
//				-1,	"CmvMatroxChannel::Open,m_pJNsdGraphStateManager==NULL!", true);
//		}
//	}
//
//	return hr;
//}

HRESULT __stdcall CmvMatroxChannel::Open
	(
	SMvResolutionInfo* in_pRes
	)
{
	HRESULT hr = MV_NOERROR;

	//if (m_pJNsdGraphStateManager == NULL)
	//{
	//	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
	//		-1,	"CmvMatroxChannel::Open,m_pJNsdGraphStateManager==NULL!", false);
	//}

	//ASSERT(sizeof(m_sMvResolution) == sizeof(*in_pRes));
	//m_sMvResolution = *in_pRes;
	SMvResolutionInfo	sMvResolution;
	//NsResTurnToMvRes(in_pRes,&sMvResolution);

	if(memcmp(&m_sMvResolution,in_pRes,sizeof(sMvResolution))!=0)
	{
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
			-1,	"memcmp(&m_sMvResolution,&sMvResolution,sizeof(sMvResolution)!=0", false);
	}

	if(m_sMvResolution.eScanMode == keMvScanModeProgressive)
	{
		m_ui32WriteToReadDelayInVideoUnit = 1;
	}
	else
	{
		m_ui32WriteToReadDelayInVideoUnit = 2;
	}

	m_ui32DepthInFrame = 32;           //Node Depth

	int nOutputConnectorCountRequired = 1;

	if(m_sMvResolution.ulWidth > 1920)
	{
		nOutputConnectorCountRequired = 4;     //UHD
	}

	_FillConnectors(nOutputConnectorCountRequired);
	_BuildPlaybackMainTopology();

	_DefineUserBufferDescription();
	_CreateVideoAudioPoolManagers();


	CreateThread();
	//try
	//{
	//	m_oThread.Create(CmvMatroxChannel::ThreadRun, this);
	//	m_oVideoThread.Create(CmvMatroxChannel::VideoThreadRun,this);
	//	m_oAudioThread.Create(CmvMatroxChannel::AudioThreadRun,this);
	//}
	//catch (...)
	//{
	//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
	//		-1,	"CmvMatroxChannel::SetClockInterface: m_oThread.Create Failed!", true);
	//}
	// check the thread create successfully, because we create thread not 
	// suspend, so we can wait here
	//DWORD dwWait = WaitForSingleObject(m_hEventThreadCreated, 5000);
	//if(dwWait != WAIT_OBJECT_0)
	//{
	//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
	//		-1,	"CmvMatroxChannel::SetClockInterface: m_hEventThreadCreated timeout!", true);
	//}

	return hr;
}

//UINT CmvMatroxChannel::ThreadRun(void* pThis)
//{
//	CmvMatroxChannel* pThread = static_cast<CmvMatroxChannel*>(pThis);
//	pThread->ThreadLoop();
//	return 0;
//}
//
//UINT CmvMatroxChannel::VideoThreadRun(void* pThis)
//{
//	CmvMatroxChannel* pThread = static_cast<CmvMatroxChannel*>(pThis);
//	pThread->VideoThreadLoop();
//	return 0;
//}
//
//UINT CmvMatroxChannel::AudioThreadRun(void* pThis)
//{
//	CmvMatroxChannel* pThread = static_cast<CmvMatroxChannel*>(pThis);
//	pThread->AudioThreadLoop();
//	return 0;
//}

HRESULT CmvMatroxChannel::_FillConnectors(int in_ConnectorCount)
{
	HRESULT		hr = MV_NOERROR;

	TMvSmartPtr<IMvSdiVideoOutputConnectorsEnumerator>	pJOutputEnum;

	hr = m_pJSystemCardConfig->CreateSdiVideoOutputConnectorsEnumerator(&pJOutputEnum);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

	for (int i=0;i<in_ConnectorCount;i++)
	{
		TMvSmartPtr<IMvSdiVideoOutputConnector> pJOutputConnector;
		hr = pJOutputEnum->Next(&pJOutputConnector);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

		if (pJOutputConnector->IsAvailable())
		{
			m_vecSdiVideoOutputConnectors.push_back(pJOutputConnector);
		}
	}

	if (in_ConnectorCount!= m_vecSdiVideoOutputConnectors.size())
	{
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,-1,
			"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );
	}

	return hr;
}

void CmvMatroxChannel::_SetOutputNodeDefaultSettings( SMvNodeSettings &out_sNodeSettings, 
	SMvNodeSettingsVideo &out_sNodeSettingsVideo,
	SMvNodeSettingsAudio &out_sNodeSettingsAudio,
	SMvNodeSettingsVanc &out_sNodeSettingsVanc)
{
	// Create a node to attach to the output streams
	out_sNodeSettings.ui32Size                         = sizeof(SMvNodeSettings);
	out_sNodeSettings.ui32Depth                        = m_ui32DepthInFrame;
	out_sNodeSettings.ui64WriteToReadDelayInNanoTime   = MvGetNanoTimeForSample(m_ui32WriteToReadDelayInVideoUnit, m_sMvResolution); 
	out_sNodeSettings.eNodeContent                     = keMvNodeContentVideoAudio/*keMvNodeContentVideoAudioVanc*/;

	sprintf_s(out_sNodeSettings.szName, _countof(out_sNodeSettings.szName), "OutputNode");
	out_sNodeSettingsVideo.ui32Size 		   = sizeof(SMvNodeSettingsVideo);
	out_sNodeSettingsVideo.sResInfo		   = m_sMvResolution;
	out_sNodeSettingsVideo.eColorimetry	   = NMvHelper::GetColorimetryFromResolution(m_sMvResolution);

	out_sNodeSettingsVideo.eSurfaceFormat	= keMvSurfaceFormatYUYV422;

	out_sNodeSettingsAudio.eChannelType				   = keChannelTypeStereo;
	out_sNodeSettingsAudio.ui32Size                 = sizeof(SMvNodeSettingsAudio);
	out_sNodeSettingsAudio.eDataType                = keAudioDataTypePCM;
	out_sNodeSettingsAudio.ui32BitsPerSample        = 32;
	out_sNodeSettingsAudio.ui32ValidBitsPerSample   = 24;

	out_sNodeSettingsVanc.eVancFormat = keMvSurfaceFormatYUYV422;
	out_sNodeSettingsVanc.ui32ComponentBitCount = 8;
	out_sNodeSettingsVanc.ui32Size = sizeof(SMvNodeSettingsVanc);
}

void CmvMatroxChannel::_SetOutputStreamDefaultSettings(SMvOutputStreamSettings & out_sOutputStreamSettings)
{
	SMvOutputStreamSettings sOutputStreamSettings =
	{
		sizeof(sOutputStreamSettings), // uint32_t ui32Size;
		// SMvAudioOutputStreamSettings sAudioOutputStreamSettings;
		{
			false, // bool bEnableAudioMixer;
				false, // bool bAudioPhaseInvert;
			{ 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 }, // double adAudioInputStreamVolume[16];
			{ 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 }, // double adAudioOutputStreamVolume[16];
			1.0, // Audio inline mixer line master volume;
			1.0, // Audio inline mixer Aux. master volume;
			keMvAudioMixerSourceNode //Audio inline mixer Aux. source selected;
		},
		// SMvVideoOutputStreamSettings sVideoOutputStreamSettings;
		{
			true,  // bool bAllowSuperBlack;
				true,  // bool bAllowSuperWhite;
				false, // bool bEnableChromaClipping;
				false, // bool bDownscaleKeyLuma;
				false, // bool bInvertKeyLuma;
				false, // bool bApplyKeyLumaOffset;
				false, // bool bEnableEdgeSharpeningFilter;
			}
	};

	sOutputStreamSettings.aeVideoConnectorPosition[0] = keMvQuadrantTopLeft;
	sOutputStreamSettings.aeVideoConnectorPosition[1] = keMvQuadrantTopRight;
	sOutputStreamSettings.aeVideoConnectorPosition[2] = keMvQuadrantBottomLeft;
	sOutputStreamSettings.aeVideoConnectorPosition[3] = keMvQuadrantBottomRight;

	out_sOutputStreamSettings = sOutputStreamSettings;
}

HRESULT CmvMatroxChannel::_BuildPlaybackMainTopology()
{

	HRESULT hr = MV_NOERROR;

	hr = m_pJSystemConfiguration->CreateSystemTopology("mvsTopologyForIOControl", m_pJClock, &m_pJSystemTopology);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxChannel::_BuildPlaybackMainTopology  error", true );

	// Output card node on IO
	SMvNodeSettings      sOutputNodeSettings;
	SMvNodeSettingsVideo sOutputVideoSettings;
	SMvNodeSettingsAudio sOutputAudioSettings;
	SMvNodeSettingsVanc  sOutputVanSettings;

	_SetOutputNodeDefaultSettings(sOutputNodeSettings, sOutputVideoSettings, sOutputAudioSettings, sOutputVanSettings);
	sprintf_s(sOutputNodeSettings.szName, _countof(sOutputNodeSettings.szName), "OutputNode");

	hr = m_pJSystemTopology->CreateCardNode(sOutputNodeSettings,
		&sOutputVideoSettings,
		&sOutputAudioSettings,
		&sOutputVanSettings,
		m_pJSystemCardConfig,
		&m_pJOutputNode);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );


	// Output stream
	SMvOutputStreamSettings sOutputStreamSettings;
	IMvVideoOutputConnector * apIVideoOutputConnetor[4];
	int nOutputConnectotNum;

	nOutputConnectotNum = m_vecSdiVideoOutputConnectors.size();

	for(int i=0; i<nOutputConnectotNum; i++)
	{
		apIVideoOutputConnetor[i] = m_vecSdiVideoOutputConnectors[i];
	}

	_SetOutputStreamDefaultSettings(sOutputStreamSettings);

	// By default, playback video on SDI output A
	hr = m_pJSystemTopology->CreateOutputStream("MainOutput", 
		m_pJOutputNode, 
		m_sMvResolution, 
		sOutputStreamSettings, 
		apIVideoOutputConnetor,
		nOutputConnectotNum,
		&m_pJOutputStream);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

	EMvAudioPair aeAudioPair[8] = {  keAudioPair0, keAudioPair1, keAudioPair2, keAudioPair3, 
		keAudioPair4, keAudioPair5, keAudioPair6, keAudioPair7 };

	// Fills in completely the SDI audio pairs
	hr =m_vecSdiVideoOutputConnectors[0]->SetSourceOutputStreamAndAudioPairs(m_pJOutputStream, aeAudioPair, 1);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

	return hr;
}

HRESULT CmvMatroxChannel::_DefineUserBufferDescription()
{
	HRESULT hr = MV_NOERROR;

	unsigned long ulUserBufferMaxSize = (m_sMvResolution.ulHeight)*(m_sMvResolution.ulWidth)* 20/8;   //YUV

	m_sUserBufferSurfaceDescription.size = sizeof(SMvUserBufferSurfaceDescription);
	m_sUserBufferSurfaceDescription.bIsVideoShaped = false;
	m_sUserBufferSurfaceDescription.bUseAlpha = false;
	m_sUserBufferSurfaceDescription.eColorimetry = keMvColorimetryITUR_BT_709;
	m_sUserBufferSurfaceDescription.eFormat = keMvSurfaceFormatYUYV422;
	m_sUserBufferSurfaceDescription.eImageOrigin = keMvImageOriginTopLeft;
	m_sUserBufferSurfaceDescription.ePolarity = keMvPolarityProgressiveFrame;
	m_sUserBufferSurfaceDescription.eType = keMvSurfaceType2D;
	m_sUserBufferSurfaceDescription.ptDestinationPosition.fX= 0.0;
	m_sUserBufferSurfaceDescription.ptDestinationPosition.fY = 0.0;
	m_sUserBufferSurfaceDescription.ulBufferSizeInBytes = 9600*2160;

	m_sUserBufferSurfaceDescription.sTemporalCompressionInfo.size = sizeof(SMvTemporalCompressionInfo);
	m_sUserBufferSurfaceDescription.sTemporalCompressionInfo.bDependedOn = false;
	m_sUserBufferSurfaceDescription.sTemporalCompressionInfo.bDependedOnValid = false;
	m_sUserBufferSurfaceDescription.sTemporalCompressionInfo.bDependsOn = false;
	m_sUserBufferSurfaceDescription.sTemporalCompressionInfo.bDependsOnValid = false;
	m_sUserBufferSurfaceDescription.sTemporalCompressionInfo.bRandomAccess = false;
	m_sUserBufferSurfaceDescription.ulComponentBitCount = m_sMvResolution.ulComponentBitCount;
	m_sUserBufferSurfaceDescription.ulDepth = 1;
	m_sUserBufferSurfaceDescription.ulHeight = m_sMvResolution.ulHeight;
	m_sUserBufferSurfaceDescription.ulWidth = m_sMvResolution.ulWidth;
	m_sUserBufferSurfaceDescription.ulRowPitchInBytes = m_sUserBufferSurfaceDescription.ulWidth * 20/8;
	m_sUserBufferSurfaceDescription.ulSlicePitchInBytes = ulUserBufferMaxSize;

	if (m_sMvResolution.ulComponentBitCount == 8)
	{
		m_sUserBufferSurfaceDescription.ulRowPitchInBytes = m_sUserBufferSurfaceDescription.ulWidth * 2;
	}

	return hr;
}

HRESULT CmvMatroxChannel::_DefineAudioSamplesDescription(SMvCreateAudioSamplesDescription * io_psAudioSamplesDescription)
{
	HRESULT hr = MV_NOERROR;

	io_psAudioSamplesDescription->size = sizeof(SMvCreateAudioSamplesDescription);

	io_psAudioSamplesDescription->bHasFadeIn = false;
	io_psAudioSamplesDescription->bHasFadeOut = false;
	io_psAudioSamplesDescription->eMemoryLocation = keMvMemoryLocationHost;
	io_psAudioSamplesDescription->ePolarity = keMvPolarityProgressiveFrame;
	io_psAudioSamplesDescription->ulMaxBufferSize = 2002 * m_eAudioChannelType * (keAudioBitsPerSample32 >> 3); 

	io_psAudioSamplesDescription->sWaveFormat.size = sizeof(SMvaWaveFormatInfo);
	io_psAudioSamplesDescription->sWaveFormat.eChannelType = m_eAudioChannelType;
	io_psAudioSamplesDescription->sWaveFormat.eDataType = keAudioDataTypePCM;
	io_psAudioSamplesDescription->sWaveFormat.ulBitsPerSample = 32;
	io_psAudioSamplesDescription->sWaveFormat.ulValidBitsPerSample = 24;
	io_psAudioSamplesDescription->sWaveFormat.ulSamplesPerSec = 48000;

	return hr;
}

HRESULT CmvMatroxChannel::_CreateVideoAudioPoolManagers()
{
	HRESULT	hr = MV_NOERROR;
	uint32_t    ui32InitialSize = 100;
	uint32_t    ui32MaxSize = 5000;

	if(m_pJSystemConfiguration == NULL)
	{
		return MV_E_FAIL;
	}

	// User buffer surface pool for capture
	hr = m_pJSystemConfiguration->CreateSurfaceUserBufferPool(L"m_pJVideoUserBufferSurfacePoolPlayback", 
		ui32InitialSize, 
		ui32MaxSize, 
		keMvScanModeProgressive, 
		&m_pJVideoUserBufferSurfacePoolForPlayback);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

	if(m_pJAudioVideoSurfaceAsynchErroCallback == NULL)
	{
		m_pJAudioVideoSurfaceAsynchErroCallback = new CNsdMyCompletionObserver();
	}
	hr = m_pJVideoUserBufferSurfacePoolForPlayback->AddCompletionObserver(m_pJAudioVideoSurfaceAsynchErroCallback);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

	//Create Audio samples pool

	SMvCreateAudioSamplesDescription sAudioSamplesDescription;
	_DefineAudioSamplesDescription(&sAudioSamplesDescription);

	hr = m_pJSystemConfiguration->CreateAudioSamplesPool(L" m_pJAudioSamplesPoolForPlayback",
		ui32InitialSize, ui32MaxSize, sAudioSamplesDescription, &m_pJAudioSamplesPoolForPlayback);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

	hr = m_pJAudioSamplesPoolForPlayback->AddCompletionObserver(m_pJAudioVideoSurfaceAsynchErroCallback);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowResourceManager,keLogPkgDirectShowResourceManagerFuncGeneral,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );

	return hr;
}


//HRESULT __stdcall CmvMatroxChannel::PlayVideoBuffer
//	(
//	INsSurface* io_pIPlaybackSurface
//	) 
//{
//	HRESULT hr = MV_NOERROR;
//
//	bool bSeeking = false;
//	if(m_pJNsdGraphStateManager)
//	{
//		hr = m_pJNsdGraphStateManager->IsSeeking(&bSeeking);
//	}
//	
//
//	CNsAutoLock lock(&m_CritSecClearBuffer);
//
//	if (bSeeking)
//	{
//		m_bClearBuffer = false;
//	}
//
//	if (m_bClearBuffer)
//	{
//		uint64_t ui64SystemTime = 0;
//		TMvSmartPtr<INsAVContent>  pJAVContent = NULL;
//		hr = io_pIPlaybackSurface->QueryInterface(IID_INsAVContent, (void**)&pJAVContent);
//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: QueryInterface failed!", true);
//
//		hr = pJAVContent->SignalReadCompletion();
//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
//
//		return hr;
//	}
//
//	io_pIPlaybackSurface->AddRef();
//	m_pListpJNsOutputSurface.AddTail(io_pIPlaybackSurface);
//
//	return hr;
//}
//
//HRESULT __stdcall CmvMatroxChannel::PlayAudioBuffer
//	(
//	unsigned long     ulStreamIndex,
//	INsAudioSamples* io_pIPlaybackAudioSample
//	) 
//{
//	HRESULT hr = MV_NOERROR;
//
//	bool bSeeking = false;
//	if(m_pJNsdGraphStateManager)
//	{
//		hr = m_pJNsdGraphStateManager->IsSeeking(&bSeeking);
//	}
//
//	CNsAutoLock lock(&m_CritSecClearBuffer);
//
//	if (bSeeking)
//	{
//		m_bClearBuffer = false;
//	}
//
//	if (m_bClearBuffer)
//	{
//		uint64_t ui64SystemTime = 0;
//		TMvSmartPtr<INsAVContent>  pJAVContent = NULL;
//		hr = io_pIPlaybackAudioSample->QueryInterface(IID_INsAVContent, (void**)&pJAVContent);
//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: QueryInterface failed!", true);
//
//		hr = pJAVContent->SignalReadCompletion();
//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
//
//		return hr;
//	}
//
//	io_pIPlaybackAudioSample->AddRef();
//	m_pListpJNsOutputAudioSamples.AddTail(io_pIPlaybackAudioSample);
//
//	return hr;
//}

HRESULT CmvMatroxChannel::VideoThreadLoop()
{
	HRESULT hr = MV_NOERROR;

	while(true)
	{
		TMvSmartPtr < IMvSurface > pJNsSurface;
		TMvSmartPtr < IMvAVContent > pJNsAVContent;
		TMvSmartPtr <IMvSurface> pJMvSurface;
		TMvSmartPtr <IMvAVContent> pJMvAVContent;
		uint64_t  ui64TimeStamp = 0;

		WaitForSingleObject((HANDLE)(m_pListpJNsOutputSurface), INFINITE);

		pJNsSurface = m_pListpJNsOutputSurface.RemoveHead();
		//pJNsSurface->Release();

		hr = pJNsSurface->QueryInterface(IID_IMvAVContent, (void**)&pJNsAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		{
			CMvAutoLock lock(&m_CritSecClearBuffer);

			if (m_bClearBuffer)
			{
				hr = pJNsAVContent->SignalReadCompletion();
				mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
					hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
				continue;
			}
		}

		// Wait for video user buffer surface from pool
		hr = m_pJVideoUserBufferSurfacePoolForPlayback->WaitForSurface(5500, &pJMvSurface);
		if (FAILED(hr))
		{
			mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
				hr, "CmvMatroxChannel::VideoThreadLoop: WaitForSurface failed, wait for ever !", true);

			hr = m_pJVideoUserBufferSurfacePoolForPlayback->WaitForSurface(INFINITE, &pJMvSurface);
			mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
				hr, "CmvMatroxChannel::VideoThreadLoop: WaitForSurface failed!", true);

			OutputDebugStringA("CmvMatroxChannel::VideoThreadLoop,WaitForSurface succeed,continue");
		}

		hr = pJMvSurface->QueryInterface(IID_IMvAVContent, (void**)&pJMvAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);	

		hr = pJMvAVContent->AttachCustomObject(&CLSID_mvMatroxDevice,pJNsSurface);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);	

		SMvLockSurfaceDescription sLockSurface = { sizeof( SMvLockSurfaceDescription )};		
		hr = pJNsSurface->Lock(0, keMvFaceTypeFront, &sLockSurface);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		m_sUserBufferSurfaceDescription.pBuffer = sLockSurface.pBuffer; 
		hr = pJMvSurface->AssignUserBufferReference(&m_sUserBufferSurfaceDescription);		
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsSurface->Unlock(0, keMvFaceTypeFront);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsAVContent->SignalReadCompletion();
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		// synchronize surface
		hr = pJNsAVContent->GetTimeStampInfo(&ui64TimeStamp);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		NMvs::PrepareSynchronization(pJMvAVContent, ui64TimeStamp);  //RC=1, WC=1
		hr = pJMvSurface->SignalWriteCompletion();			// RC=1, WC=0.
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		CMvAutoLock lock(&m_CritSecClearBuffer);

		if (!m_bClearBuffer)
		{
			m_List_VideoSurface.AddTail(pJMvSurface);
			//pJMvSurface->AddRef(); 
		}
	}

	return hr;
}

HRESULT CmvMatroxChannel::AudioThreadLoop()
{
	HRESULT hr = MV_NOERROR;

	while(true)
	{
		TMvSmartPtr < IMvAudioSamples > pJNsAudioSamples;
		TMvSmartPtr < IMvAVContent > pJNsAVContent;
		TMvSmartPtr <IMvAudioSamples> pJMvAudioSamples;
		TMvSmartPtr <IMvAVContent> pJMvAVContent;
		uint64_t  ui64TimeStamp = 0;

		WaitForSingleObject((HANDLE)(m_pListpJNsOutputAudioSamples), INFINITE);

		pJNsAudioSamples = m_pListpJNsOutputAudioSamples.RemoveHead();
		//pJNsAudioSamples->Release();

		hr = pJNsAudioSamples->QueryInterface(IID_IMvAVContent, (void**)&pJNsAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		{
			CMvAutoLock lock(&m_CritSecClearBuffer);

			if (m_bClearBuffer)
			{
				hr = pJNsAVContent->SignalReadCompletion();
				mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
					hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
				continue;
			}
		}

		// Get audio sample from pool
		hr = m_pJAudioSamplesPoolForPlayback->WaitForAudioSamples(5500, &pJMvAudioSamples);
		if (FAILED(hr))
		{
			mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
				hr, "CmvMatroxChannel::AudioThreadLoop: WaitForAudioSamples failed!", true);

			hr = m_pJAudioSamplesPoolForPlayback->WaitForAudioSamples(INFINITE, &pJMvAudioSamples);
			mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
				hr, "CmvMatroxChannel::AudioThreadLoop: WaitForAudioSamples failed!", true);

			OutputDebugStringA("CmvMatroxChannel::AudioThreadLoop,WaitForAudioSamples succeed,continue");
		}


		hr = pJMvAudioSamples->QueryInterface(IID_IMvAVContent, (void**)&pJMvAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		/*		hr = pJMvAVContent->AttachCustomObject(&CLSID_mvMatroxDevice,pJNsAudioSamples);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);	*/	

		//hr = io_pJPlaybackAudioSamples->ChangePolarity(keMvPolarityProgressiveFrame);

		// Set audio sample buffer size in bytes. This is required if the audio sample pool is retrieved from IMvSystemConfiguration
		//ulAudioSampleValidBufferLength = ulNumberOfSamples* m_eAudioChannelType * (keAudioBitsPerSample32 >> 3);
		//hr = io_pJPlaybackAudioSamples->SetLength(ulAudioSampleValidBufferLength);
		unsigned char* pDstBuf = NULL;
		unsigned long ulDstlength;
		hr =pJMvAudioSamples->GetBufferAndLength((void**)&pDstBuf,&ulDstlength);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		unsigned char* pSrcBuf = NULL;
		unsigned long ulSrclength;
		hr =  pJNsAudioSamples->GetBufferAndLength((void**)&pSrcBuf,&ulSrclength);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		//ASSERT(ulDstlength>=ulSrclength);

		hr = pJMvAudioSamples->SetLength(ulSrclength);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		unsigned long ulSampleCount = 0;
		hr = pJNsAudioSamples->GetValidBufferLengthInSamples(&ulSampleCount);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		SMvAudioSamplesDescription sDescSrcAudio;
		sDescSrcAudio.size = sizeof(SMvAudioSamplesDescription);
		hr = pJNsAudioSamples->GetAudioSamplesDescription(&sDescSrcAudio);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		if(sDescSrcAudio.sWaveFormat.ulBitsPerSample == 32)//32=>32
		{
			memcpy(pDstBuf,pSrcBuf,ulSrclength);
		}
		else if(sDescSrcAudio.sWaveFormat.ulBitsPerSample == 16)//16=>32
		{
			int16_t *pi16Src = (int16_t *)pSrcBuf;
			int32_t *pi32Dst = (int32_t *)pDstBuf;
			for (unsigned long ulIdx = 0; ulIdx < ulSampleCount; ulIdx++)
			{
				for (int i = 0; i < sDescSrcAudio.sWaveFormat.eChannelType; i++)
					*(pi32Dst++) = (int32_t)((*(pi16Src++)) << 16);
			}
		}

		// synchronize surface
		hr = pJNsAVContent->GetTimeStampInfo(&ui64TimeStamp);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		NMvs::PrepareSynchronization(pJMvAVContent, ui64TimeStamp);  //RC=1, WC=1
		hr = pJMvAudioSamples->SignalWriteCompletion();			// RC=1, WC=0.
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsAVContent->SignalReadCompletion();
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		CMvAutoLock lock(&m_CritSecClearBuffer);

		if (!m_bClearBuffer)
		{
			m_List_AudioSample.AddTail(pJMvAudioSamples);
			//pJMvAudioSamples->AddRef();
		}
	}
	return hr;
}

HRESULT CmvMatroxChannel::ThreadLoop()
{
	HRESULT hr = MV_NOERROR;

	bool bContinue = true;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	// 有三个事件需要等待
	HANDLE hWaitHandle[1];
	hWaitHandle[0] = m_hsemaphoreClock;

	// for clock use
	__int64 rtNowTime;
	__int64 rtPeriod;

	hr = GetTime((int64_t*)&rtNowTime);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);

	//if (m_pJRefClock)
	//{
	//	hr = m_pJRefClock->GetTime(&rtNowTime);
	//	mv_SAFE_REPORT_ERROR_RETURN( keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,hr,
	//		"CmvMatroxChannel::ThreadLoop",true);
	//}
	//else
	//{
	//	rtNowTime = m_pJClock->GetTime();
	//}
	
	
	uint64_t ui64TimeInFrame = MvGetSampleForNanoTime( rtNowTime,m_sMvResolution );

	rtNowTime = MvGetNanoTimeForSample( ui64TimeInFrame , m_sMvResolution );


	rtPeriod = (__int64)(MvGetNanoTimeForSample( 1 , m_sMvResolution ));

	// 启动 Clock Thread
	//if (m_pJRefClock)
	//{
	//	hr = m_pJRefClock->AdvisePeriodic(rtNowTime, rtPeriod, reinterpret_cast<HSEMAPHORE>(m_hsemaphoreClock), &m_ulClockCookie);
	//	mv_SAFE_REPORT_ERROR_RETURN( keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,hr,
	//		"CmvMatroxChannel::ThreadLoop",true);
	//}
	//else
	{
		hr = m_oCmvMatroxDevice->AddSemaphore(m_hsemaphoreClock);
	}
	
	const int COUNT = 999999;
	CNsHighResTimer HighResTimerMixLyrics;

	__int64 i64MixLyrics = COUNT;
	int nMixLyricsMin = COUNT;
	int nMixLyricsMax = COUNT;

	 int nVideoMinCount = COUNT;
	 int nVideoMaxCount = COUNT;
	 int nAudioMinCount = COUNT;
	 int nAudioMaxCount = COUNT;

	 __int64 i64Differ = COUNT;
	 int nDifferMin = COUNT;
	 int nDifferMax = COUNT;

	 int nCount = 0;
	
	
	

	//SetEvent(m_hEventThreadCreated);

	while(bContinue)
	{

		DWORD dwWait = WaitForMultipleObjects(1, hWaitHandle, FALSE, INFINITE);

		if(dwWait == WAIT_OBJECT_0)
		{
			while(true)
			{
				if (m_bExist)
				{
					return hr;
				}

				if (m_List_VideoSurface.GetCount() <= 0)
				{
					CMvAutoLock lock(&m_CritSecClearBuffer);
					if (m_bClearBuffer && m_List_AudioSample.GetCount()>0)
					{
						ClearBufferAt(0,FALSE,"ThreadLoop");
						CMvAutoLock lock1(&m_CritSecLyricsSurface);
						m_pJLyricsSurface = NULL;
					}
					break;
				}

				TMvSmartPtr<IMvSurface> pJSurface;
				pJSurface = m_List_VideoSurface.GetHead();

				uint64_t ui64SurTime = GetVidoSurfaceTimeStamp(pJSurface);
				uint64_t ui64SurTimeInFrame = MvGetSampleForNanoTime( ui64SurTime, m_sMvResolution );

				uint64_t ui64CurrentTime=0;

				hr = GetTime((int64_t*)&ui64CurrentTime);
				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
					hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);

				//if (m_pJRefClock)
				//{
				//	hr = m_pJRefClock->GetTime ((__int64 *)&ui64CurrentTime);
				//	mv_SAFE_REPORT_ERROR_RETURN( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
				//		hr, "CmvMatroxChannel::ThreadLoop GetTime failed",true);
				//}
				//else
				//{
				//	ui64CurrentTime = m_pJClock->GetTime();
				//}
				

				uint64_t ui64CurrentTimeInFrame = MvGetSampleForNanoTime( ui64CurrentTime, m_sMvResolution );

				 i64Differ = ui64SurTimeInFrame - ui64CurrentTimeInFrame;

				if (nDifferMax<i64Differ || nDifferMax == COUNT)
				{
					nDifferMax = i64Differ;
				}
				if (nDifferMin>i64Differ || nDifferMin == COUNT)
				{
					nDifferMin = i64Differ;
				}

				if(i64Differ>4)
				{
					break;
				}

				uint64_t ui64SystemTime = ui64SurTime;
				TMvSmartPtr<IMvAVContent>  pJAVContent = NULL;
				hr = pJSurface->QueryInterface(IID_IMvAVContent, (void**)&pJAVContent);
				mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
					hr, "CNsdStreamBasedPlayBack::ThreadLoop GetTime failed",true);

				//ui64SystemTime = ui64SurTime - MvGetNanoTimeForSample(1,m_sMvResolution);

				hr = pJAVContent->SetTimeStampInfo(&ui64SystemTime);
				mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
					hr, "CNsdStreamBasedPlayBack::ThreadLoop GetTime failed",true);


				bool bSeeking = false;
				//if(m_pJNsdGraphStateManager)
				//{
				//	hr = m_pJNsdGraphStateManager->IsSeeking(&bSeeking);
				//	mv_SAFE_REPORT_ERROR_RETURN( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
				//		hr, "CmvMatroxChannel::ThreadLoop GetTime failed",true);
				//}
				

				if(bSeeking)
				{
					{
						static int count=0;
						++count;
						char temp[200];
						sprintf(temp,"f_ySeek,CmvMatroxChannel::ThreadLoop,GetCount=(v=%d,a=%d),count=%d",
							m_List_VideoSurface.GetCount(),m_List_AudioSample.GetCount(),count);
						OutputDebugString(temp);
					}
				}

				//{
				//	static int count=0;
				//	++count;
				//	char temp[200];
				//	sprintf(temp,"zmbCancel,Video,CmvMatroxChannel::ThreadLoop,%I64d,%I64d,GetCount=%d,count=%d",
				//		ui64SurTimeInFrame*2,ui64SurTime/10000,m_List_VideoSurface.GetCount(),count);
				//	OutputDebugString(temp);
				//}

				try
				{
					
					HighResTimerMixLyrics.Reset();
					MixLyrics(pJSurface);
					i64MixLyrics= HighResTimerMixLyrics.GetElapseInMicroSecond();

					hr = PlayVideoAudioBufferToDevice();
					mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
						hr,	"CNsd4KOutputKernal::ThreadLoop: Fail to MakeSyncReceiveBuffer!", true);
				}
				catch (...)
				{
					mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
						-1,	"CNsd4KOutputKernal::ThreadLoop: catch error !", true);
				}

			}

			if (nMixLyricsMax<i64MixLyrics|| nMixLyricsMax == COUNT)
			{
				nMixLyricsMax = i64MixLyrics;
			}
			if (nMixLyricsMin>i64MixLyrics|| nMixLyricsMin == COUNT)
			{
				nMixLyricsMin = i64MixLyrics;
			}

			if (nVideoMaxCount<m_List_VideoSurface.GetCount() || nVideoMaxCount == COUNT)
			{
				nVideoMaxCount = m_List_VideoSurface.GetCount();
			}
			if (nVideoMinCount>m_List_VideoSurface.GetCount() || nVideoMinCount == COUNT)
			{
				nVideoMinCount = m_List_VideoSurface.GetCount();
			}
			if (nAudioMaxCount<m_List_AudioSample.GetCount() || nAudioMaxCount == COUNT)
			{
				nAudioMaxCount = m_List_AudioSample.GetCount();
			}
			if (nAudioMinCount>m_List_AudioSample.GetCount() || nAudioMinCount == COUNT)
			{
				nAudioMinCount = m_List_AudioSample.GetCount();
			}

			nCount++;
			if (nCount%500==0)
			{
				char temp[400];
				sprintf_s(temp,"f_y,MatroxChannel::ThreadLoop,ID=%d,video=(%d,%d,%d),audio=(%d,%d,%d),Differ=(%I64d,%d,%d),Lyrics=(%I64d,%d,%d),Nsbuffer(v=%d,a=%d),count=%d",
					m_nChannelID,
					m_List_VideoSurface.GetCount(),nVideoMinCount,nVideoMaxCount,
					m_List_AudioSample.GetCount(),nAudioMinCount,nAudioMaxCount,
					i64Differ,nDifferMin,nDifferMax,
					i64MixLyrics,nMixLyricsMin,nMixLyricsMax,
					m_pListpJNsOutputSurface.GetCount(),m_pListpJNsOutputAudioSamples.GetCount(),nCount);
				OutputDebugString(temp);

				nVideoMinCount = COUNT;
				nVideoMaxCount = COUNT;
				nAudioMinCount = COUNT;
				nAudioMaxCount = COUNT;

				nDifferMax = COUNT;
				nDifferMin = COUNT;
				i64Differ = COUNT;

				i64MixLyrics = COUNT;
				nMixLyricsMin = COUNT;
				nMixLyricsMax = COUNT;
				//nCount = 0;
			}
		}
		//else if(dwWait == WAIT_OBJECT_0 + 1)
		//{
		//	// kill thread
		//	bContinue = false;
		//}
	}

	// exit we reset create event
	//ResetEvent(m_hEventThreadCreated);

	//if (m_pJRefClock)
	//{
	//	m_pJRefClock->Unadvise(m_ulClockCookie);
	//	m_ulClockCookie = 0;
	//}
	
	return MV_NOERROR;
}

HRESULT CmvMatroxChannel::GetTime(int64_t * out_pTime)
{
	HRESULT hr = MV_NOERROR;

	//if (m_pJRefClock)
	//{
	//	hr = m_pJRefClock->GetTime(out_pTime);
	//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
	//		hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);
	//}
	//else
	{
		*out_pTime = m_pJClock->GetTime();
	}

	return hr;
}

HRESULT CmvMatroxChannel::PlayVideoAudioBufferToDevice()
{
	HRESULT hr = NOERROR;

	bool bSeeking = false;
	//if(m_pJNsdGraphStateManager)
	//{
	//	hr = m_pJNsdGraphStateManager->IsSeeking(&bSeeking);
	//	mv_SAFE_REPORT_ERROR_RETURN( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
	//		hr, "CmvMatroxChannel::ThreadLoop GetTime failed",true);
	//}

	//CNsAutoLock lock(&m_oReceiveBuffer);
	//{
	//	static int count=0;
	//	++count;
	//	char temp[200];
	//	sprintf(temp,"zmb8,MakeSyncReceiveBuffer,Video=%d,Audio=%d",
	//		m_List_VideoSurface.GetCount(),m_List_AudioSample[0].GetCount());
	//	OutputDebugString(temp);
	//}

	if (m_List_VideoSurface.GetCount() <= 0)
	{
		return hr;
	}

	if(!bSeeking && !m_bClearBuffer)
	{
		if(m_List_AudioSample.GetCount() <= 0)
		{
			mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
				-1,	"CmvMatroxChannel::ThreadLoop: Fail to MakeSyncReceiveBuffer!", false);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	if ( m_sMvResolution.eScanMode == keMvScanModeFirstFieldTop || 
		m_sMvResolution.eScanMode == keMvScanModeSecondFieldTop )
	{
		// list内至少有两场数据
		if (m_List_VideoSurface.GetCount() <= 1)
		{
			return hr;
		}

		if(!bSeeking)
		{
			if(m_List_AudioSample.GetCount() <= 1)
			{
				mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
					-1,	"CmvMatroxChannel::ThreadLoop: Fail to MakeSyncReceiveBuffer!", false);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		TMvSmartPtr<IMvSurface> pJSurfaceF0 = NULL;
		TMvSmartPtr<IMvSurface> pJSurfaceF1 = NULL;
		TMvSmartPtr<IMvAudioSamples> pJSamplesF0 = NULL;
		TMvSmartPtr<IMvAudioSamples> pJSamplesF1 = NULL;

		// video
		TMvSmartPtr<IMvSurface> pJSurface = NULL;
		pJSurface = m_List_VideoSurface.RemoveHead();
		//pJSurface->Release();

		SMvSurfaceDescription sSurfaceDescription;
		sSurfaceDescription.size = sizeof(SMvSurfaceDescription);
		hr = pJSurface->GetSurfaceDescription(&sSurfaceDescription);
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
			hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: pJSamplesF0->ChangePolarity Failed!", true);

		if (sSurfaceDescription.ePolarity == keMvPolarityFirstField)
		{
			pJSurfaceF0 = pJSurface;
			pJSurfaceF1 = m_List_VideoSurface.RemoveHead();
			//pJSurfaceF1->Release();            
		}
		else if (sSurfaceDescription.ePolarity == keMvPolaritySecondField)
		{
			pJSurfaceF1 = pJSurface;
			pJSurfaceF0 = m_List_VideoSurface.RemoveHead();
			//pJSurfaceF0->Release();
		}
		else
		{
			mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
				-1,	"CmvMatroxChannel::ThreadLoop: Fail to MakeSyncReceiveBuffer!", true);
		}

		// audio
		TMvSmartPtr<IMvAudioSamples> pJSamples;
		pJSamples = m_List_AudioSample.RemoveHead();

		if(pJSamples != NULL)
		{
			//pJSamples->Release();
			SMvAudioSamplesDescription sAudioSamplesDescription;
			sAudioSamplesDescription.size = sizeof(SMvAudioSamplesDescription);
			hr = pJSamples->GetAudioSamplesDescription(&sAudioSamplesDescription);
			mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
				hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: pJSamplesF0->ChangePolarity Failed!", true);

			if (sAudioSamplesDescription.ePolarity == keMvPolarityFirstField)
			{
				pJSamplesF0 = pJSamples;
				pJSamplesF1 = m_List_AudioSample.RemoveHead();
				if(pJSamplesF1 != NULL)
				{
					//pJSamplesF1->Release();
					hr = pJSamplesF1->ChangePolarity(keMvPolaritySecondField);
					mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
						hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: pJSamplesF1->ChangePolarity Failed!", true);
				}
			}
			else if (sAudioSamplesDescription.ePolarity == keMvPolaritySecondField)
			{
				pJSamplesF1 = pJSamples;
				pJSamplesF0 = m_List_AudioSample.RemoveHead();
				if(pJSamplesF0 != NULL)
				{
					//pJSamplesF0->Release();
					hr = pJSamplesF0->ChangePolarity(keMvPolarityFirstField);
					mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
						hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: pJSamplesF0->ChangePolarity Failed!", true);
				}

			}
			else
			{
				mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
					-1,	"CmvMatroxChannel::ThreadLoop: Fail to MakeSyncReceiveBuffer!", true);
			}
		}


		//////////////////////////////////////////////////////////////////////////
		//         TMvSmartPtr<INsAVContent> pJAVContent0 = NULL;
		//         TMvSmartPtr<INsAVContent> pJAVContent1 = NULL;
		//         hr = pJSurfaceF0->QueryInterface(IID_INsAVContent, (void**)&pJAVContent0);
		//         hr = pJSurfaceF1->QueryInterface(IID_INsAVContent, (void**)&pJAVContent1);
		// 
		//         unsigned long ulReadCountF0 = 0;
		//         unsigned long ulWriteCountF0 = 0;
		//         uint64_t ui64TimestampF0 = 0;
		//         hr = pJAVContent0->GetReadCount(&ulReadCountF0);
		//         hr = pJAVContent0->GetWriteCount(&ulWriteCountF0);
		//         hr = pJAVContent0->GetTimeStampInfo(&ui64TimestampF0);
		//         HRESULT hrTempF0 = MV_NOERROR;
		//         hr = pJAVContent0->GetLastError(&hrTempF0);
		//         ASSERT(SUCCEEDED(hrTempF0));
		// 
		//         unsigned long ulReadCountF1 = 0;
		//         unsigned long ulWriteCountF1 = 0;
		//         uint64_t ui64TimestampF1 = 0;
		//         hr = pJAVContent1->GetReadCount(&ulReadCountF1);
		//         hr = pJAVContent1->GetWriteCount(&ulWriteCountF1);
		//         hr = pJAVContent1->GetTimeStampInfo(&ui64TimestampF1);
		//         HRESULT hrTempF1 = MV_NOERROR;
		//         hr = pJAVContent1->GetLastError(&hrTempF1);
		//         ASSERT(SUCCEEDED(hrTempF1));
		// 
		//         uint64_t ui64CurTime = 0;
		//         hr = m_pJRefClock->GetTime((__int64 *)&ui64CurTime);
		// 
		//         char temp[100];
		//         sprintf(temp, "ywx, ui64TimestampF0=%I64u, ui64TimestampF1=%I64u, ui64CurTime=%I64u, ", ui64TimestampF0, ui64TimestampF1, ui64CurTime);
		//         OutputDebugString(temp);
		// 
		// 
		// 
		//         hr = pJAVContent0->SignalReadCompletion();
		//         hr = pJAVContent1->SignalReadCompletion();
		//         pJSurfaceF0 = NULL;
		//         pJSurfaceF1 = NULL;

		//////////////////////////////////////////////////////////////////////////


		//if(m_pJDevicePlaybackNotify != NULL)
		//{
		//	TMvSmartPtr<INsAVContent> pJAVContent = NULL;
		//	hr = pJSurfaceF1->QueryInterface(IID_INsAVContent, (void**)&pJAVContent);
		//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//		hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: QueryInterface Failed!", true);

		//	TMvSmartPtr<IUnknown> pJUnkStreamingObject = NULL;
		//	hr = pJAVContent->GetCustomObject( &IID_INsdStreamingObject, &pJUnkStreamingObject);
		//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//		hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: GetCustomObject Failed!", true);

		//	TMvSmartPtr<INsdStreamingObject> pJStreamingObject = NULL;
		//	hr = pJUnkStreamingObject->QueryInterface( IID_INsdStreamingObject, ( void **)&pJStreamingObject );
		//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//		hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: QueryInterface Failed!", true);

		//	SNsdStreamingInfo *psStreamingInfo = NULL;
		//	hr = pJStreamingObject->GetStreamingInfo( &psStreamingInfo );
		//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//		hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: GetStreamingInfo Failed!", true);

		//	//             if( m_pJDevicePlaybackNotify != NULL )
		//	{
		//		TMvSmartPtr<INsdLayerStreamingObject> pJLayerStreamingObject = NULL;
		//		hr = pJStreamingObject->QueryInterface(IID_INsdLayerStreamingObject, (void**)&pJLayerStreamingObject);
		//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//			hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: QueryInterface Failed!", true);

		//		unsigned long ulSourceStreamingLayerCount = 0;
		//		hr = pJLayerStreamingObject->GetLayerCount( &ulSourceStreamingLayerCount );
		//		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//			hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: GetLayerCount Failed!", true);

		//		SNsStreamingState * psStreamingState = NULL ;
		//		for( unsigned long ulIndex = 0; ulIndex < ulSourceStreamingLayerCount; ulIndex ++ )
		//		{
		//			hr = pJLayerStreamingObject->GetLayerStreamingStateByIndex(ulIndex, &psStreamingState );
		//			mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//				hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: GetLayerStreamingStateByIndex Failed!", true);

		//			hr = m_pJDevicePlaybackNotify->NotifyDevicePlayback( ulIndex, psStreamingState );
		//			mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//				hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: NotifyDevicePlayback Failed!", true);
		//		}

		//		//avoid the matrox play buffer fail : e30df014(audio) and e30df114(video)
		//		psStreamingInfo->ui64TimelinePosInEres = psStreamingInfo->ui64StreamTimeInEres;
		//	}
		//}
		//////////////////////////////////////////////////////////////////////////

		TMvSmartPtr<IMvAVContent> pJAVContentVideo0 = NULL;
		TMvSmartPtr<IMvAVContent> pJAVContentVideo1 = NULL;
		hr = pJSurfaceF0->QueryInterface(IID_IMvAVContent, (void**)&pJAVContentVideo0);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
			"CmvMatroxChannel::MakeSyncReceiveBuffer   QueryInterface failed", true );

		hr = pJSurfaceF1->QueryInterface(IID_IMvAVContent, (void**)&pJAVContentVideo1);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
			"CmvMatroxChannel::MakeSyncReceiveBuffer   QueryInterface failed", true );

		TMvSmartPtr<IMvAVContent> pJAVContentAudio0 = NULL;
		TMvSmartPtr<IMvAVContent> pJAVContentAudio1 = NULL;

		if(pJSamplesF0 != NULL)
		{
			hr = pJSamplesF0->QueryInterface(IID_IMvAVContent, (void**)&pJAVContentAudio0);
			mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
				"CmvMatroxChannel::MakeSyncReceiveBuffer   QueryInterface failed", true );
		}

		if(pJSamplesF1 != NULL)
		{
			hr = pJSamplesF1->QueryInterface(IID_IMvAVContent, (void**)&pJAVContentAudio1);
			mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
				"CmvMatroxChannel::MakeSyncReceiveBuffer   QueryInterface failed", true );
		}


		//if(bSeeking)
		{
			bool bWaste = false;
			if(pJSamplesF0 == NULL && pJAVContentAudio1!=NULL )
			{
				bWaste = true;
				hr = pJAVContentAudio1->SignalReadCompletion();
				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
					"CmvMatroxChannel::MakeSyncReceiveBuffer(),SignalReadCompletion", true );
				pJSamplesF1 = NULL;
			}
			else if(pJSamplesF1 == NULL && pJAVContentAudio0!=NULL )
			{
				bWaste = true;
				hr = pJAVContentAudio0->SignalReadCompletion();
				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
					"CmvMatroxChannel::MakeSyncReceiveBuffer(),SignalReadCompletion", true );
				pJSamplesF0 = NULL;
			}

			if(pJAVContentAudio0!=NULL)
			{
				uint64_t ui64SurTime = GetVidoSurfaceTimeStamp(pJSurfaceF0);
				hr = pJAVContentAudio0->SetTimeStampInfo(&ui64SurTime);
				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
					"CmvMatroxChannel::MakeSyncReceiveBuffer(),SetTimeStampInfo", true );
			}

			if(pJAVContentAudio1!=NULL)
			{
				uint64_t ui64SurTime = GetVidoSurfaceTimeStamp(pJSurfaceF1);
				hr = pJAVContentAudio1->SetTimeStampInfo(&ui64SurTime);
				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowNodes,keLogPkgDirectShowNodesFuncGeneral,hr,
					"CmvMatroxChannel::MakeSyncReceiveBuffer(),SetTimeStampInfo", true );
			}

			if(bWaste)
			{
				static int count=0;
				++count;
				char temp[200];
				sprintf(temp,"f_y,bWaste,CmvMatroxChannel::PlayVideoAudioBufferToDevice,GetCount=(%d,%d),count=%d",
					m_List_VideoSurface.GetCount(),m_List_AudioSample.GetCount(),count);
				OutputDebugString(temp);
			}

		}

		hr = m_pJOutputNode->SetNodeContentFields( pJSurfaceF0, 
			pJSurfaceF1, 
			pJSamplesF0, 
			pJSamplesF1, 
			NULL, 
			NULL);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
			hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);
	}
	else
	{
		TMvSmartPtr<IMvSurface> pJSurface;
		TMvSmartPtr<IMvAVContent> pJAVContentVideo;
		TMvSmartPtr<IMvAudioSamples> pJSamples;
		TMvSmartPtr<IMvAVContent> pJAVContentAudio;

		pJSurface = m_List_VideoSurface.RemoveHead();
		//pJSurface->Release();

		hr = pJSurface->QueryInterface(IID_IMvAVContent, (void**)&pJAVContentVideo);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule,keLogPkgDirectShowDSXLEModule,hr,
			"CmvMatroxChannel::MakeSyncReceiveBuffer   QueryInterface failed", true );

		//SMvSurfaceDescription sSurfaceDescription;
		//sSurfaceDescription.size = sizeof(SMvSurfaceDescription);
		//hr = pJSurface->GetSurfaceDescription(&sSurfaceDescription);
		//mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		//	hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: pJSamplesF0->ChangePolarity Failed!", true);

		pJSamples = m_List_AudioSample.RemoveHead();

		if(pJSamples != NULL)
		{
			//pJSamples->Release();
			//SMvAudioSamplesDescription sAudioSamplesDescription;
			//sAudioSamplesDescription.size = sizeof(SMvAudioSamplesDescription);
			//hr = pJSamples->GetAudioSamplesDescription(&sAudioSamplesDescription);
			//mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
			//	hr,	"CmvMatroxChannel::MakeSyncReceiveBuffer: pJSamplesF0->ChangePolarity Failed!", true);

			hr = pJSamples->QueryInterface(IID_IMvAVContent, (void**)&pJAVContentAudio);
			mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule,keLogPkgDirectShowDSXLEModule,hr,
				"CNsd4KOutputKernal::MakeSyncReceiveBuffer   QueryInterface failed", true );
		}

		uint64_t ui64SurTime = 0;
		ui64SurTime = GetVidoSurfaceTimeStamp(pJSurface);

		//if(bSeeking)
		{
			if(pJAVContentAudio!=NULL)
			{
				hr = pJAVContentAudio->SetTimeStampInfo(&ui64SurTime);
				mv_SAFE_REPORT_ERROR_RETURN(0,0,hr,
					"CNsd4KOutputKernal::MakeSyncReceiveBuffer(),SetTimeStampInfo", true );
			}
		}

		hr = m_pJOutputNode->SetNodeContentFrame(pJSurface,pJSamples,NULL);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
			hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);

		if (bSeeking)
		{
			uint64_t ui64CurrentTime=0;

			hr = GetTime((int64_t*)&ui64CurrentTime);
			mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
				hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);

			static int count=0;
			++count;
			char temp[400];
			sprintf_s(temp,"f_ySeek,PlayVideoAudioBufferToDevice,after SetNodeContentFrame,GetCount=(v=%d,a=%d),SurTime-CurrentTime=(%I64d-%I64d)Nano=%I64dms,count=%d",
				m_List_VideoSurface.GetCount(),m_List_AudioSample.GetCount(),ui64SurTime,ui64CurrentTime,(ui64SurTime-ui64CurrentTime)/10000,count);
			OutputDebugString(temp);
		}
	}
	return hr;
}

//HRESULT CmvMatroxChannel::PlayAt(__int64 in_rtTime)
//{	
//	CNsAutoLock lock(&m_CritSecClearBuffer);
//	ClearBufferAt(0, FALSE,"PlayAt");
//	m_bClearBuffer = false;
//	return NOERROR;
//}
//
//HRESULT CmvMatroxChannel::ClearAllVideo()
//{	
//	ClearBufferAt(0, TRUE,"ClearAllVideo");
//	return NOERROR;
//}
//
//HRESULT CmvMatroxChannel::ClearAllAudio()
//{	
//	ClearBufferAt(0, FALSE,"ClearAllAudio");
//	return NOERROR;
//}
//
//HRESULT CmvMatroxChannel::ClearAtVideo(__int64 in_rtClear)
//{	
//	ClearBufferAt(in_rtClear, TRUE,"ClearAtVideo");
//	return NOERROR;
//}
//
//HRESULT CmvMatroxChannel::ClearAtAudio(__int64 in_rtClear)
//{	
//	ClearBufferAt(in_rtClear, FALSE,"ClearAtAudio");
//	return NOERROR;
//}



HRESULT  CmvMatroxChannel::ClearBufferAt(__int64 in_rtClear, BOOL bVideo,char* pszFlag)
{
	return MV_NOERROR;
}

//{	
//
//	CMvAutoLock lock(&m_CritSecClearBuffer);
//
//	m_bClearBuffer = true;
//
//	HRESULT hr = NOERROR;
//
//	TMvSmartPtr<IMvSurface> pJPlayBackSur;
//	TMvSmartPtr<IMvAudioSamples> pJPlayBackSample;
//
//	uint64_t ui64ClearPosInFrame = MvGetSampleForNanoTime(in_rtClear, m_sMvResolution);
//
//	uint64_t ui64CurTime = 0;
//
//	hr = GetTime((int64_t*)&ui64CurTime);
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
//		hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);
//
//	//if (m_pJRefClock)
//	//{
//	//	hr = m_pJRefClock->GetTime((__int64 *)&ui64CurTime);
//	//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
//	//		hr,	"CNsdStreamBasedTools::SetNodeContentFields SetNodeContentFields Failed!", true);
//	//}
//	//else
//	//{
//	//	ui64CurTime = m_pJClock->GetTime();
//	//}
//	
//
//	//uint64_t ui64TimeStamp = MvGetSampleForNanoTime(ui64CurTime, m_sMvResolution);
//
//	{
//		static int count=0;
//		++count;
//		char temp[300];
//		sprintf(temp,"f_yCancel,CmvMatroxChannel::ClearBufferAt,flag=%s,Clear(%I64d) - CurTime(%I64d)=%I64dms,GetCount()=(Video=%d,Audio=%d),bVideo=%d,count=%d",
//			pszFlag,in_rtClear/10000,ui64CurTime/10000,in_rtClear/10000 - ui64CurTime/10000,m_List_VideoSurface.GetCount(),m_List_AudioSample.GetCount(),bVideo,count);
//		OutputDebugString(temp);
//	}
//
//
//	if(bVideo)
//	{
//		//POSITION pos = m_List_VideoSurface.GetHeadPosition();
//		while(m_List_VideoSurface.GetCount()!=0)
//		{
//			//POSITION posTemp = pos;
//			pJPlayBackSur = m_List_VideoSurface.GetNext(pos);
//			uint64_t ui64SurTime = GetVidoSurfaceTimeStamp(pJPlayBackSur);
//			uint64_t ui64SurTimeInFrame = MvGetSampleForNanoTime(ui64SurTime, m_sMvResolution);
//
//			//uint64_t ui64SurTimeInField = 0;
//			//if(m_sMvResolution.eScanMode == keNsScanModeProgressive || m_sMvResolution.eScanMode == keNsScanModeProgressiveSegmented)
//			//{
//			//	ui64SurTimeInField = MvGetSampleForNanoTime( ui64SurTime, m_sMvResolution );
//			//}
//			//else
//			//{
//			//	ui64SurTimeInField = CNsdTime::Nano2Field( ui64SurTime, m_sMvResolution );
//			//}
//
//			//note: in field mode, the ui64ClearPos is frame conunt,but the ui64SurTimeInField is filed count,so the 
//			//if(ui64SurTimeInField >= ui64ClearPos)// If( ui64SurTimeInField >= ui64ClearPos ) is always true .
//			//在720p 或1080 p时，都在帧模式下，这段代码起作用，但会导致部分buffer 没有播出去且没有被release,导致泄露，
//			//在stop时 ECComplete 不能回去，音频亦然。所有注掉 lqz 2011.6.21
//			if(ui64SurTimeInFrame >= ui64ClearPosInFrame)
//			{
//				TMvSmartPtr<IMvAVContent>  pJAVContent;
//				hr = pJPlayBackSur->QueryInterface(IID_IMvAVContent, (void**)&pJAVContent);
//				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
//					hr,	"CmvMatroxChannel::ClearBufferAt: SignalReadCompletion Failed!", true);
//
//				hr = pJAVContent->SignalReadCompletion();
//				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
//					hr,	"CmvMatroxChannel::ClearBufferAt: SignalReadCompletion Failed!", true);
//
//				m_List_VideoSurface.Remove(posTemp);
//
//				//pJPlayBackSur->Release();
//			}
//			else
//			{
//				{
//					static int count=0;
//					++count;
//					char temp[300];
//					sprintf(temp,"f_yCancel,Video left,CmvMatroxChannel::ClearBufferAt,flag=%s,ClearPos(%I64d) - TimeStamp(%I64d)=%I64dFrame,ClearPos=%I64dms,count=%d",
//						pszFlag,ui64ClearPosInFrame,ui64SurTimeInFrame,ui64ClearPosInFrame - ui64SurTimeInFrame,in_rtClear/10000,count);
//					OutputDebugString(temp);
//				}
//			}
//
//
//
//			//pJPlayBackSur = NULL;
//
//		}
//
//	}
//
//	if(!bVideo)
//	{
//		POSITION pos = m_List_AudioSample.GetHeadPosition();
//		while(pos != NULL)
//		{
//			POSITION posTemp = pos;
//			pJPlayBackSample = m_List_AudioSample.GetNext(pos);
//			uint64_t ui64SamTime = GetAudioSampleTimeStamp(pJPlayBackSample);
//			uint64_t ui64SamTimeInFrame = MvGetSampleForNanoTime(ui64SamTime, m_sMvResolution);
//
//			//uint64_t ui64SamTimeInField = 0;
//			//if(m_sMvResolution.eScanMode == keNsScanModeProgressive || m_sMvResolution.eScanMode == keNsScanModeProgressiveSegmented)
//			//{
//			//	ui64SamTimeInField = MvGetSampleForNanoTime( ui64SamTime, m_sMvResolution );
//			//}
//			//else
//			//{
//			//	ui64SamTimeInField = CNsdTime::Nano2Field( ui64SamTime, m_sMvResolution );
//			//}
//
//
//			if(/*true ||*/ ui64SamTimeInFrame >= ui64ClearPosInFrame)
//			{
//
//				m_List_AudioSample.Remove(posTemp);
//
//				TMvSmartPtr<IMvAVContent>  pJAVContent = NULL;
//				hr = pJPlayBackSample->QueryInterface(IID_IMvAVContent, (void**)&pJAVContent);
//				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
//					hr,	"CmvMatroxChannel::ClearBufferAt: SignalReadCompletion Failed!", true);
//
//				hr = pJAVContent->SignalReadCompletion();
//				mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
//					hr,	"CmvMatroxChannel::ClearBufferAt: SignalReadCompletion Failed!", true);
//
//				//pJPlayBackSample->Release();
//			}
//			else
//			{
//				{
//					static int count=0;
//					++count;
//					char temp[300];
//					sprintf(temp,"f_yCancel,Audio left,,CmvMatroxChannel::ClearBufferAt,flag=%s,ClearPos(%I64d) - TimeStamp(%I64d)=%I64dFrame,ClearPos=%I64dms,count=%d",
//						pszFlag,ui64ClearPosInFrame,ui64SamTimeInFrame,ui64ClearPosInFrame - ui64SamTimeInFrame,in_rtClear/10000,count);
//					OutputDebugString(temp);
//				}
//			}
//
//		}
//
//	}
//
//	return hr;
//}

HRESULT CmvMatroxChannel::MixLyrics(IMvSurface* in_pIVideoSurface)
{
	return MV_NOERROR;
}
//{
//	HRESULT hr = MV_NOERROR;
//
//	TMvSmartPtr<INsSurface> pJLyricsSurface;
//
//	{
//		CNsAutoLock lock(&m_CritSecLyricsSurface);
//		if (m_pJLyricsSurface == NULL)
//		{
//			return hr;
//		}
//		pJLyricsSurface = m_pJLyricsSurface ;
//		//m_pJLyricsSurface = NULL;
//	}
//	
//	TMvSmartPtr <IMvAVContent> pJMvAVContent;
//	TMvSmartPtr< IUnknown > pJUnkSurface;
//	TMvSmartPtr < INsSurface > pJNsSurface;
//	TMvSmartPtr<INsAVContent>  pJNsAVContent;
//	TMvSmartPtr< IUnknown > pJUnkStreamingObject;
//	TMvSmartPtr< INsdStreamingObject > pJStreamingObject;
//	SNsdStreamingInfo sStreamingInfo = { sizeof( SNsdStreamingInfo ) };
//	
//	hr = in_pIVideoSurface->QueryInterface(IID_IMvAVContent, (void**)&pJMvAVContent);
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);	
//
//	hr = pJMvAVContent->GetCustomObject(&CLSID_mvMatroxDevice,&pJUnkSurface);
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);	
//
//	hr = pJUnkSurface->QueryInterface(IID_INsSurface, (void**)&pJNsSurface);
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);	
//
//	hr = pJNsSurface->QueryInterface(IID_INsAVContent, (void**)&pJNsAVContent);
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);	
//
//	hr = pJNsAVContent ->GetCustomObject( &IID_INsdStreamingObject,&pJUnkStreamingObject );
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,
//		hr, "CNsLiveWindowThread::MixRecodCGBufferWithHostBuffer() failed!", true);
//
//	hr = pJUnkStreamingObject->QueryInterface( IID_INsdStreamingObject ,( void **)&pJStreamingObject );
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,
//		hr, "CNsLiveWindowThread::MixRecodCGBufferWithHostBuffer() failed!", true);
//
//	hr = pJStreamingObject ->GetStreamingInfoCopy( &sStreamingInfo );
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,
//		hr, "CNsLiveWindowThread::MixRecodCGBufferWithHostBuffer() failed!", true);
//
//	uint64_t ui64CurrentPosition = sStreamingInfo.ui64TimelinePosInEres;
//	//if(ui64CurrentPosition >= 1)
//	//{
//	//	ui64CurrentPosition = ui64CurrentPosition - 1; 
//	//}
//
//	//TMvSmartPtr< IUnknown > pJUnkGraphStateManagerTimingAndPositionAccess;
//
//	//hr = pJNsAVContent ->GetCustomObject( &IID_INsdGraphStateManagerTimingAndPositionAccess,&pJUnkGraphStateManagerTimingAndPositionAccess );
//	//mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,
//	//	hr, "CNsLiveWindowThread::MixRecodCGBufferWithHostBuffer failed!", true);
//
//	TMvSmartPtr< INsdGraphStateManagerTimingAndPositionAccess > pJGraphStateManagerTimingAndPositionAccess;
//
//	hr = m_pJNsdGraphStateManager->QueryInterface( IID_INsdGraphStateManagerTimingAndPositionAccess ,( void **)&pJGraphStateManagerTimingAndPositionAccess );
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,
//		hr, "CNsLiveWindowThread::MixRecodCGBufferWithHostBuffer failed!", true);
//
//	hr = pJGraphStateManagerTimingAndPositionAccess->SetCurrentLiveWindowMixingField( ui64CurrentPosition );
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowLiveWindowManager, keLogPkgDirectShowLiveWindowManagerFuncConsumeThread,
//		hr, "CNsLiveWindowThread::MixRecodCGBufferWithHostBuffer failed!", true);
//
//
//	SNsLockSurfaceDescription sLockSurface = { sizeof( SNsLockSurfaceDescription )};		
//	hr = pJNsSurface->Lock(0, keNsFaceTypeFront, &sLockSurface);
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
//
//	SNsLockSurfaceDescription sLockLyricsSurface = { sizeof( SNsLockSurfaceDescription )};		
//	hr = pJLyricsSurface->Lock(0, keNsFaceTypeFront, &sLockLyricsSurface);
//	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
//
//
//
//	hr = pJNsSurface->Unlock(0, keNsFaceTypeFront);
//	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
//
//	hr = pJLyricsSurface->Unlock(0, keNsFaceTypeFront);
//	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
//		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);
//
//	return hr;
//}

HRESULT CmvMatroxChannel::CreateThread()
{
	HRESULT hr = MV_NOERROR;
	try
	{
		m_oThread.Create(CmvMatroxChannel::ThreadRun, this);
		m_oVideoThread.Create(CmvMatroxChannel::VideoThreadRun,this);
		m_oAudioThread.Create(CmvMatroxChannel::AudioThreadRun,this);
	}
	catch (...)
	{
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDSXLEModule, 0,
			-1,	"CmvMatroxChannel::SetClockInterface: m_oThread.Create Failed!", true);
	}

	return hr;
}

UINT CmvMatroxChannel::ThreadRun(void* pThis)
{
	CmvMatroxChannel* pThread = static_cast<CmvMatroxChannel*>(pThis);
	pThread->ThreadLoop();
	return 0;
}

UINT CmvMatroxChannel::VideoThreadRun(void* pThis)
{
	CmvMatroxChannel* pThread = static_cast<CmvMatroxChannel*>(pThis);
	pThread->VideoThreadLoop();
	return 0;
}

UINT CmvMatroxChannel::AudioThreadRun(void* pThis)
{
	CmvMatroxChannel* pThread = static_cast<CmvMatroxChannel*>(pThis);
	pThread->AudioThreadLoop();
	return 0;
}

HRESULT __stdcall CmvMatroxChannel::PlayVideoBuffer
	(
	IMvSurface* io_pIPlaybackSurface
	) 
{
	HRESULT hr = MV_NOERROR;

	bool bSeeking = false;
	//if(m_pJNsdGraphStateManager)
	//{
	//	hr = m_pJNsdGraphStateManager->IsSeeking(&bSeeking);
	//}


	CMvAutoLock lock(&m_CritSecClearBuffer);

	if (bSeeking)
	{
		m_bClearBuffer = false;
	}

	if (m_bClearBuffer)
	{
		uint64_t ui64SystemTime = 0;
		TMvSmartPtr<IMvAVContent>  pJAVContent = NULL;
		hr = io_pIPlaybackSurface->QueryInterface(IID_IMvAVContent, (void**)&pJAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: QueryInterface failed!", true);

		hr = pJAVContent->SignalReadCompletion();
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		return hr;
	}

	//io_pIPlaybackSurface->AddRef();
	m_pListpJNsOutputSurface.AddTail(io_pIPlaybackSurface);

	return hr;
}

HRESULT __stdcall CmvMatroxChannel::PlayAudioBuffer
	(
	unsigned long     ulStreamIndex,
	IMvAudioSamples* io_pIPlaybackAudioSample
	) 
{
	HRESULT hr = MV_NOERROR;

	bool bSeeking = false;
	//if(m_pJNsdGraphStateManager)
	//{
	//	hr = m_pJNsdGraphStateManager->IsSeeking(&bSeeking);
	//}

	CMvAutoLock lock(&m_CritSecClearBuffer);

	if (bSeeking)
	{
		m_bClearBuffer = false;
	}

	if (m_bClearBuffer)
	{
		uint64_t ui64SystemTime = 0;
		TMvSmartPtr<IMvAVContent>  pJAVContent = NULL;
		hr = io_pIPlaybackAudioSample->QueryInterface(IID_IMvAVContent, (void**)&pJAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: QueryInterface failed!", true);

		hr = pJAVContent->SignalReadCompletion();
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		return hr;
	}

	//io_pIPlaybackAudioSample->AddRef();
	m_pListpJNsOutputAudioSamples.AddTail(io_pIPlaybackAudioSample);

	return hr;
}

HRESULT CmvMatroxChannel::PlayAt(__int64 in_rtTime)
{	
	CMvAutoLock lock(&m_CritSecClearBuffer);
	ClearBufferAt(0, FALSE,"PlayAt");
	m_bClearBuffer = false;
	return NOERROR;
}

HRESULT CmvMatroxChannel::ClearAllVideo()
{	
	ClearBufferAt(0, TRUE,"ClearAllVideo");
	return NOERROR;
}

HRESULT CmvMatroxChannel::ClearAllAudio()
{	
	ClearBufferAt(0, FALSE,"ClearAllAudio");
	return NOERROR;
}

HRESULT CmvMatroxChannel::ClearAtVideo(__int64 in_rtClear)
{	
	ClearBufferAt(in_rtClear, TRUE,"ClearAtVideo");
	return NOERROR;
}

HRESULT CmvMatroxChannel::ClearAtAudio(__int64 in_rtClear)
{	
	ClearBufferAt(in_rtClear, FALSE,"ClearAtAudio");
	return NOERROR;
}