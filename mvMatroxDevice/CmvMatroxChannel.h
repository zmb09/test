#pragma once
#include "mvMatroxDevice.h"
#include "CmvMatroxCallBack.h"
#include "CmvMatroxDevice.h"

class CmvMatroxDevice;

class CmvMatroxChannel :public CMvUnknown
{
public:
	MVDECLARE_IUNKNOWN;
	CmvMatroxChannel(LPUNKNOWN in_poUnknown=0, HRESULT* out_pHresult=0);
	public:
	virtual ~CmvMatroxChannel();
public:


public:


	virtual HRESULT __stdcall Open
		(
		SMvResolutionInfo* in_pRes
		);

	virtual HRESULT __stdcall PlayVideoBuffer
		(
		IMvSurface* io_pIPlaybackSurface
		);

	virtual HRESULT __stdcall PlayAudioBuffer
		(
		unsigned long     ulStreamIndex,
		IMvAudioSamples* io_pIPlaybackAudioSample
		);

	virtual HRESULT __stdcall ClearAllVideo();

	virtual HRESULT __stdcall ClearAllAudio();

	virtual HRESULT __stdcall ClearAtVideo
		(
		__int64 in_rtClear
		);

	virtual HRESULT __stdcall ClearAtAudio
		(
		__int64 in_rtClear
		);

	virtual HRESULT __stdcall PlayAt
		(
		__int64 in_rtTime
		);

	HRESULT GetTime(int64_t * out_pTime);
private:
	HRESULT MixLyrics(IMvSurface* in_pIVideoSurface);
private:
	HRESULT ClearBufferAt(__int64 in_rtClear, BOOL bVideo,char *pszFlag="");

private:
	HRESULT _FillConnectors(int in_ConnectorCount);
	HRESULT _BuildPlaybackMainTopology();
	void _SetOutputNodeDefaultSettings( SMvNodeSettings &out_sNodeSettings, 
		SMvNodeSettingsVideo &out_sNodeSettingsVideo,
		SMvNodeSettingsAudio &out_sNodeSettingsAudio,
		SMvNodeSettingsVanc &out_sNodeSettingsVanc);
	void _SetOutputStreamDefaultSettings(SMvOutputStreamSettings & out_sOutputStreamSettings);
	HRESULT _DefineUserBufferDescription();
	HRESULT      _DefineAudioSamplesDescription(SMvCreateAudioSamplesDescription * io_psAudioSamplesDescription);
	HRESULT _CreateVideoAudioPoolManagers();

	HRESULT PlayVideoAudioBufferToDevice();
	
public:
	//int m_ichannelID;
	TMvSmartPtr<IMvSystemConfiguration>		m_pJSystemConfiguration;
	TMvSmartPtr<IMvCardConfiguration>		m_pJSystemCardConfig;
	TMvSmartPtr<IMvSystemClock>		m_pJClock;
	//std::vector<EMvVideoConnectorLabel>  m_vecVideoConnectorLabel;
	std::vector<TMvSmartPtr<IMvSdiVideoOutputConnector> > m_vecSdiVideoOutputConnectors;

	SMvResolutionInfo	m_sMvResolution;

private:
	 
	TMvSmartPtr<IMvSystemTopology>			m_pJSystemTopology;
	uint32_t m_ui32WriteToReadDelayInVideoUnit;
	uint32_t m_ui32DepthInFrame;
	TMvSmartPtr<IMvNode>				m_pJOutputNode;
	TMvSmartPtr<IMvOutputStream>		m_pJOutputStream;

	SMvUserBufferSurfaceDescription                 m_sUserBufferSurfaceDescription;
	TMvSmartPtr<IMvSurfacePool>                     m_pJVideoUserBufferSurfacePoolForPlayback;
	TMvSmartPtr<IMvAudioSamplesPool>                m_pJAudioSamplesPoolForPlayback;
	TMvSmartPtr<CNsdMyCompletionObserver>   m_pJAudioVideoSurfaceAsynchErroCallback;
	EMvChannelType		m_eAudioChannelType;

private:
	CMyHeadTailSignalList <TMvSmartPtr<IMvSurface> >  m_List_VideoSurface;
	CMyHeadTailSignalList <TMvSmartPtr<IMvAudioSamples> >   m_List_AudioSample;

private:
	//TMvSmartPtr<INsGraphStateManager> m_pJNsdGraphStateManager;
	//TMvSmartPtr<IReferenceClock> m_pJRefClock;
	//SNsResolutionInfo m_sMvResolution;
	//CNsdHeadTailSignalList <INsSurface> m_pListpJNsOutputSurface;
	//CNsdHeadTailSignalList <INsAudioSamples> m_pListpJNsOutputAudioSamples;

//	CNsCriticalSection   m_CritSecClearBuffer;
//	bool m_bClearBuffer;
//	static UINT __stdcall VideoThreadRun(void* pThis);
	HRESULT VideoThreadLoop();
//
//	static UINT __stdcall AudioThreadRun(void* pThis);
	HRESULT AudioThreadLoop();
//	
//	static UINT __stdcall ThreadRun(void* pThis);
	HRESULT ThreadLoop();
//private:
//	CNsThread m_oAudioThread;
//	CNsThread m_oVideoThread;
//	CNsThread m_oThread;
//	//HANDLE m_hEventThreadCreated;
//	HANDLE m_hsemaphoreClock;
//	DWORD_PTR m_ulClockCookie;
//	bool m_bExist;
public:
	CmvMatroxDevice * m_oCmvMatroxDevice;
	//int m_nChannelID;


	protected:
		//TNsSmartPtr<INsGraphStateManager> m_pJNsdGraphStateManager;
		//TNsSmartPtr<IReferenceClock> m_pJRefClock;
		//SNsResolutionInfo m_sMvResolution;

		CMyHeadTailSignalList <TMvSmartPtr<IMvSurface> >m_pListpJNsOutputSurface;
		CMyHeadTailSignalList <TMvSmartPtr<IMvAudioSamples> > m_pListpJNsOutputAudioSamples;

		CMvCriticalSection   m_CritSecClearBuffer;
		bool m_bClearBuffer;
		static UINT __stdcall VideoThreadRun(void* pThis);
		//virtual HRESULT VideoThreadLoop();

		static UINT __stdcall AudioThreadRun(void* pThis);
		//virtual HRESULT AudioThreadLoop();

		static UINT __stdcall ThreadRun(void* pThis);
		//virtual HRESULT ThreadLoop();

		CMvThread m_oAudioThread;
		CMvThread m_oVideoThread;
		CMvThread m_oThread;
		//HANDLE m_hEventThreadCreated;
		HANDLE m_hsemaphoreClock;
		DWORD_PTR m_ulClockCookie;
		bool m_bExist;

		CMvCriticalSection   m_CritSecLyricsSurface;
		TMvSmartPtr<IMvSurface> m_pJLyricsSurface;

public:
	int m_nChannelID;
protected:
	HRESULT CreateThread();
	//virtual HRESULT ClearBufferAt(__int64 in_rtClear, BOOL bVideo,char *pszFlag="") = 0;
	//virtual HRESULT PlayVideoAudioBufferToDevice() = 0;
	//HRESULT GetTime(int64_t * out_pTime);


};



