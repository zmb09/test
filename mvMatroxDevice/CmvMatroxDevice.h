#pragma once
#include "mvMatroxDevice.h"
#include "CmvMatroxCallBack.h"
#include "CmvMatroxChannel.h"

class CmvMatroxChannel;
//enum EStreamingEvent
//{
//	keStreamingEventEndThread,
//	keStreamingEventStream,
//	keStreamingEventStop,
//	keStreamingEventCount
//};
//
//enum EDefaultStateEvent
//{
//	keDefaultStateWrittenTopology,
//	keDefaultStateClearedFlash,
//	keDefaultStateWrittenCount
//};

class CmvMatroxDevice :public CMvUnknown,
						public IMvSystemClockNotificationCallback
{
public:
	CmvMatroxDevice(LPUNKNOWN in_poUnknown=0, HRESULT* out_pHresult=0);
public:
	virtual ~CmvMatroxDevice();
public:
	MVDECLARE_IUNKNOWN;
public:
	// System clock notify
	virtual void __stdcall NotifyClock();

	HRESULT Init( SMvResolutionInfo* in_psResolutionInfo);
	//HRESULT GetClockEvent(HANDLE *out_hClockEvent);
	//HRESULT GetClock(INsdClock **out_ppINsdClock);
	HRESULT AddSemaphore(HANDLE in_hSemaphore);
	HRESULT AddEvent(HANDLE in_hEvent);
	HRESULT GetChannelByIndex(CmvMatroxChannel** out_ppNsdOutputDeviceControl,int *io_pchannelID=0);
public:	//INsdClock
	HRESULT __stdcall GetCurrentTimes( uint64_t *out_pui64Time );
	//virtual HRESULT __stdcall GetEventHandle( HANDLE * out_phEvent );
protected:
	HRESULT CreateChannel(CmvMatroxChannel** out_ppNsdOutputDeviceControl);
private:

	HRESULT _DestroyAllPreviousSystemTopology();
	HRESULT _SetGenlockClock();
	HRESULT _SetupGenlockResolution();
	HRESULT _SetSystemClockCallback();
	uint64_t _GetNextClockTime();

public:
	TMvSmartPtr<IMvSystemConfiguration>		m_pJSystemConfiguration;
	TMvSmartPtr<IMvCardConfiguration>		m_pJSystemCardConfig;
	

private:
	//HANDLE  m_ahDefaultStateSignaling[keDefaultStateWrittenCount];
	SMvResolutionInfo	m_sResolution; 
	//HANDLE m_hClockEvent;
	//bool m_bGetClockEvent;
	//CNsCriticalSection   m_CritSecHandle;
	//std::vector<HANDLE> m_vecSemaphore;
	//std::vector<HANDLE> m_vecEvent;
	//Asynch error notification
	TMvSmartPtr<IMvErrorNotification>			m_pJErrorNotification;
	TMvSmartPtr<IMvErrorNotificationCallback>	m_pJErrorCallback;

	TMvSmartPtr<IMvSystemClock>		m_pJClock;
protected:
	HRESULT CreateClockThread();
	static UINT __stdcall ClockThreadRun(void* pThis);
	virtual HRESULT ClockThreadLoop();
	void Signal();
	CMvThread m_oClockThread;
protected:
	HANDLE m_hClockEvent;
	bool m_bGetClockEvent;

	CMvCriticalSection   m_CritSecHandle;
	std::vector<HANDLE> m_vecSemaphore;
	std::vector<HANDLE> m_vecEvent;

	//typedef std::map< int, TNsSmartPtr<INsdOutputDeviceControl> >		OutputDeviceControl_MAP;
	//typedef std::map< int, TNsSmartPtr<INsdOutputDeviceControl> >::iterator	OutputDeviceControl_MAP_ITR;
	CMvCriticalSection								  m_CritSecOutputDeviceControl;
	//CNsAutoLock lock(&m_CritSecOutputDeviceControl);
	std::vector<TMvSmartPtr<CmvMatroxChannel> > m_vector_pJOutputDeviceControl;
	//OutputDeviceControl_MAP m_map_OutputDeviceControl;

};