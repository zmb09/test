#pragma once
#include "mvMatroxDevice.h"
#include "CmvMatroxDevice.h"

class CmvMatroxDeviceManager : public CMvUnknown
{
public:
	CmvMatroxDeviceManager(LPUNKNOWN in_poUnknown=0, HRESULT* out_pHresult=0);
	MVDECLARE_IUNKNOWN
	virtual ~CmvMatroxDeviceManager();

public:
	static CMvUnknown* __stdcall CreateInstance
		(
		LPUNKNOWN  in_pUnkOuter,
		HRESULT    *out_phr 
		); 

	//init Func
	static void __stdcall DllInit(bool in_bIsLoading, const CLSID *in_psClsID);
	virtual HRESULT __stdcall Init( SMvResolutionInfo* in_psResolutionInfo);
	HRESULT __stdcall GetDeviceCount(int *out_CardCount);
	//HRESULT __stdcall GetClockByIndex(INsdClock **out_ppINsdClock,int in_cardID);
	HRESULT __stdcall AddSemaphoreByIndex(HANDLE in_hSemaphore,int in_cardID);
	HRESULT __stdcall AddEventByIndex(HANDLE in_hEvent,int in_cardID);

	HRESULT __stdcall GetOutputDeviceByIndex
		(
		CmvMatroxChannel** out_ppNsdOutputDeviceControl,
		int *io_pchannelID=0,
		int in_cardID=0
		);

	TMvSmartPtr<IMvSystemConfiguration>		m_pJSystemConfiguration;
private:
	
	std::vector<TMvSmartPtr<CmvMatroxDevice> > m_vector_Device;
};
// eof