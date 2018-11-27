#pragma once
#include "mvMatroxDevice.h"


class CSystemTopologyErrorCallback : public CMvUnknown,  public IMvSystemTopologyErrorCallback//, public IMvEventNotificationCallback
{
public:
	CSystemTopologyErrorCallback(void);
	virtual ~CSystemTopologyErrorCallback(void);

	//// Interface IMvGenlockNotificationCallback
	//virtual HRESULT __stdcall Notify(HRESULT in_hrError);

	// Interface IMvSystemTopologyErrorCallback
	virtual HRESULT __stdcall NotifyAsyncError(const SMvAsyncErrorInformation& in_krsAsynErrorInfo);

	//void SetResolution(SMvResolutionInfo in_sResolution);

	//SMvResolutionInfo m_sResolution;

	// CMvUnknow implementation
	MVDECLARE_IUNKNOWN;
};

class CErrorNotificationCallback : public CMvUnknown, public IMvErrorNotificationCallback
{
public:
	CErrorNotificationCallback(void);
	virtual ~CErrorNotificationCallback(void);

	// Interface IMvGenlockNotificationCallback
	virtual HRESULT __stdcall Notify(HRESULT in_hrError);

	// CMvUnknow implementation
	MVDECLARE_IUNKNOWN;
};


class CGenlockStatusCallback : public CMvUnknown, public IMvGenlockNotificationCallback
{
public:
	CGenlockStatusCallback();

	virtual ~CGenlockStatusCallback(void);

	// Interface IMvGenlockNotificationCallback
	virtual HRESULT __stdcall Notify(SMvGenlockInputStatus& in_rsGenlockStatus);

	// CMvUnknow implementation
	MVDECLARE_IUNKNOWN;

	// Public methods
	void GetStatus(SMvGenlockInputStatus& io_rsGenlockStatus);

protected:
	CMvCriticalSection    m_csStatusLock;
	SMvGenlockInputStatus m_sGenlockStatus;
};

// Video surface/Audio samples asynch error observer
class CNsdMyCompletionObserver : 
	public IMvSurfacePoolCallback, 
	public IMvAudioSamplesPoolCallback, 
	public CMvUnknown
{
public:
	MVDECLARE_IUNKNOWN;
	CNsdMyCompletionObserver():CMvUnknown(NULL,NULL){}
	virtual ~CNsdMyCompletionObserver (){}

	virtual void    __stdcall ReceiveSurface(IMvSurface*  in_pISurface)                               //Asynch error on video surface 
	{
		HRESULT  hr = MV_NOERROR;
		HRESULT  hr_error;   
		hr = in_pISurface->GetLastError(&hr_error); 
		if(FAILED(hr_error))
		{
			char temp[256];	
			sprintf(temp,"ReceiveSurface error! hr_error=%X",hr_error);
			mv_SAFE_REPORT_ERROR(0, 0,	-1, temp, false);
		}
	}
	virtual void    __stdcall ReceiveAudioSamples(IMvAudioSamples* in_pIAudioSamples)                 //Asynch error on audio samples
	{
		HRESULT  hr = MV_NOERROR;
		HRESULT  hr_error;
		hr = in_pIAudioSamples->GetLastError(&hr_error);
		if(FAILED(hr_error))
		{
			char temp[256];	
			sprintf(temp,"ReceiveAudioSamples error! hr_error=%X",hr_error);
			mv_SAFE_REPORT_ERROR(0, 0,	-1, temp, false);
		}
	}

};