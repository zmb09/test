
#pragma once

//DSX LE SDK
#include "MvFlexGuids.h"
#include "MvFlexApi.h"
#include "mvErrors.h"
#include "mvKernelErrors.h"
#include "mvdDef.h"
#include "mvdIDS.h"
#include "mvTimeSrv.h"
#include "mvAsyncEventManagerIF.h"
#include "MatroxDSXsdk.h"
#include "mvHelperFunctions.h"
#include "mvHelperClassesAPI.h"
#include "mvsCommon.h"

#define mvAudioTime_h
#define mvTime_h
#define mvAlign_h

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:


// Windows Header Files:
#include <windows.h>

#include "assert.h"	

////#define ASSERT assert
//#include <NsDebug.h>
//
//#include <streams.h>
//#include "nsdIDs.h"
//#include "nsdAPI.h"
//#include "nsErrors.h"              // For NS error codes
//#include "TMvSmartPtr.h"
//#include "nsSTLAlloc.h"
//#include <nsTypes.h>
//#include "tchar.h"
//#include "nsSourceUtils.h"
//#include "nsPoolManagerAPI.h"
//
//#include "nsLogUserAPI.h"
//#include "nsdErrorReporting.h"
//
//#include "nsdOperatorOverLoad.h"
//
//#include "CNsdGlobalUtilityFunctions.h"
//#include "CNsdPoolElementAllocator.h"
//
//#include "NsAVFunc.h"
//#include "NsdTools.h"

#include <map>
#include <list>
#include <deque>
#include <vector>
#include <queue>

//#include <dvdmedia.h>
//#include "mtype.h"

#define keLogPkgDirectShowDSXLEModule 0
#define keLogPkgDirectShowDSXLEModuleFuncCapture 0
#define keLogPkgDirectShowDeckLinkModule 0
#define keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice 0

inline void mv_LOG_MODULE_NOTIFY(LPCSTR pszFormat, ...)
{
	int nLogBufferSize=1024, nCount;
	char pLogBuffer[1024];
	va_list argList;

	va_start(argList, pszFormat);
	nCount = _vsnprintf(pLogBuffer, nLogBufferSize, pszFormat, argList);
	va_end(argList);

	if (nCount < 0 || nCount == nLogBufferSize) {
		nCount = nLogBufferSize - 1;
		pLogBuffer[nCount] = '\0';
	}
	OutputDebugStringA(pLogBuffer);
}
//#define mv_SAFE_REPORT_ERROR_RETURN(ePackage, lFunctionality, hr, msg, bAssert) 
#define mv_SAFE_REPORT_ERROR_RETURN(ePackage, lFunctionality,hr, msg, bAssert) \
{\
	if (FAILED(hr)) { \
	mv_LOG_MODULE_NOTIFY( "%s f_y,hr=%x.File : %s. LINE : %d", msg, hr, __FILE__,__LINE__); \
	if (bAssert) ASSERT(FALSE); \
	return hr; \
	} \
}

#define mv_SAFE_REPORT_ERROR(hr, msg, bAssert) \
{\
	if (FAILED(hr)) { \
	mv_LOG_MODULE_NOTIFY( "%s f_y,hr=%x.File : %s. LINE : %d", msg, hr, __FILE__,__LINE__); \
	if (bAssert) ASSERT(FALSE); \
	} \
}


//
//#include "nsAudioTime.h"
//#include "CNsdTime.h"
//#include "NsTime.h"


//inline HRESULT NsResTurnToMvRes(SNsResolutionInfo* in_psNsResInfo,SMvResolutionInfo* out_psMvResInfo)
//{
//	HRESULT hr = MV_NOERROR;
//
//	SMvResolutionInfo sMvResolutionInfo;
//
//	sMvResolutionInfo.size = sizeof(SMvResolutionInfo);
//	sMvResolutionInfo.eScanMode = (EMvScanMode)in_psNsResInfo->eScanMode;
//	sMvResolutionInfo.ulComponentBitCount = in_psNsResInfo->ulComponentBitCount;
//	sMvResolutionInfo.eFrameRate = (EMvFrameRate)in_psNsResInfo->eFrameRate;
//	sMvResolutionInfo.ulWidth = in_psNsResInfo->ulWidth;
//	sMvResolutionInfo.ulHeight = in_psNsResInfo->ulHeight;
//	sMvResolutionInfo.eAspectRatio = (EMvAspectRatio)in_psNsResInfo->eAspectRatio;
//
//	*out_psMvResInfo = sMvResolutionInfo;
//
//	return hr;
//}

inline uint64_t GetVidoSurfaceTimeStamp(IMvSurface * pSurface)
{
	HRESULT hr = NOERROR;
	uint64_t ui64SystemTime = 0;
	TMvSmartPtr<IMvAVContent>  pJAVContent = NULL;
	hr = pSurface->QueryInterface(IID_IMvAVContent, (void**)&pJAVContent);
	mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		hr, "CNsdStreamBasedPlayBack::ThreadLoop GetTime failed",true);

	hr = pJAVContent->GetTimeStampInfo(&ui64SystemTime);
	mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		hr, "CNsdStreamBasedPlayBack::ThreadLoop GetTime failed",true);

	return ui64SystemTime;

}

inline uint64_t GetAudioSampleTimeStamp(IMvAudioSamples* pSample)
{
	HRESULT hr = NOERROR;
	uint64_t ui64SystemTime = 0;
	TMvSmartPtr<IMvAVContent>  pJAVContent = NULL;
	hr = pSample->QueryInterface(IID_IMvAVContent, (void**)&pJAVContent);
	mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		hr, "CNsdStreamBasedPlayBack::ThreadLoop GetTime failed",true);

	hr = pJAVContent->GetTimeStampInfo(&ui64SystemTime);
	mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture,
		hr, "CNsdStreamBasedPlayBack::ThreadLoop GetTime failed",true);

	return ui64SystemTime;
}



template <class T_ITEM>
class CMyHeadTailSignalList 
{
	// FALSE ,The list is in singlling event state when the list is not empty.
	// TRUE  The list is in singlling event state when it is emepty
	BOOL m_bEmptySetEvent;
	HANDLE m_hEvent;
	CMvCriticalSection   m_lock;
	std::deque<T_ITEM> m_deque;
public:

	// Cast to HANDLE - we don't support this as an lvalue
	operator HANDLE () const { return m_hEvent; };

	void Set() {SetEvent(m_hEvent);};
	BOOL Wait(DWORD dwTimeout = INFINITE) {
		return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0);
	};
	void Reset() { ResetEvent(m_hEvent); };
	BOOL Check() { return Wait(0); };	 

	int GetCount(){CMvAutoLock l(&m_lock);return m_deque.size();}
	T_ITEM GetHead()
	{
		CMvAutoLock l(&m_lock);
		return m_deque.front();
	}
public:

	CMyHeadTailSignalList (BOOL bEmptySetEvent = FALSE ,
		TCHAR *pName = NULL ,
		INT iItems = 2000,
		BOOL bLock = TRUE,
		BOOL bAlert = FALSE)
		:m_bEmptySetEvent( bEmptySetEvent )
	{
		m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL); 

		if( m_bEmptySetEvent)
		{
			Set();
		}
		else 
		{
			Reset(); // start form 0 item.
		}
	};


	T_ITEM RemoveHead() 
	{ 
		CMvAutoLock l(&m_lock);	
		T_ITEM  pObj =m_deque.front();
		m_deque.pop_front();
		if (m_deque.size() == 0)
		{
			if( m_bEmptySetEvent )
			{
				Set();
			}
			else
			{
				Reset(); // reset if is empty
			}
		}

		return pObj;
	};

	//T_ITEM *Remove( POSITION p )
	//{
	//	CMvAutoLock l(&m_lock);
	//	T_ITEM * pObj = CGenericList<T_ITEM>::Remove( p );
	//	if (GetCount() == 0)
	//	{
	//		if( m_bEmptySetEvent )
	//		{
	//			Set();
	//		}
	//		else
	//		{
	//			Reset(); // reset if is empty
	//		}
	//	}
	//	return pObj;
	//};

	T_ITEM RemoveTail() 
	{ 
		CMvAutoLock l(&m_lock);
		T_ITEM  pObj = m_deque.back();
		m_deque.pop_back(); 
		if (m_deque.size() == 0)
		{
			if( m_bEmptySetEvent )
			{
				Set();
			}
			else
			{
				Reset(); // reset if is empty
			}
		}

		return pObj;
	};

	/*POSITION*/ void AddHead(T_ITEM  pObj) 
	{ 
		CMvAutoLock l(&m_lock);
		m_deque.push_front(pObj);
		if (m_deque.size() == 1)
		{
			if( m_bEmptySetEvent )
			{
				Reset();
			}
			else
			{
				Set(); // reset if is empty
			}
		}
		//return NOERROR;
	};

	void AddTail(T_ITEM  pObj)  
	{ 
		CMvAutoLock l(&m_lock);
		m_deque.push_back(pObj);
		if (m_deque.size() == 1)
		{
			if( m_bEmptySetEvent )
			{
				Reset();
			}
			else
			{
				Set(); // reset if is empty
			}
		}
		//return NOERROR;
	};
};


class CNsHighResTimer
{
public:
	// Constructor
	CNsHighResTimer()
	{
		m_bFlag = QueryPerformanceFrequency(( LARGE_INTEGER*)&m_ui64freq);
		Reset();
	}

	// Destructor
	~CNsHighResTimer()
	{

	}

	// Get the elapse since the timer is reset(in milliseconds)
	DWORD GetElapse()
	{
		DWORD dwRet;
		if (m_bFlag)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&m_ui64Elapse);
			dwRet = (DWORD)(((m_ui64Elapse - m_ui64BeginCount) * 1000) / m_ui64freq);
		}
		else
		{
			dwRet = GetTickCount() - (DWORD)m_ui64BeginCount;
		}
		return dwRet;
	}

	UINT64 GetElapseInMicroSecond()
	{
		if (m_bFlag) {
			// we ues double variable to calculate the result
			// in order to avoid overflowing
			QueryPerformanceCounter((LARGE_INTEGER*)&m_ui64Elapse);
			return (UINT64)(((double)(m_ui64Elapse - m_ui64BeginCount) * 1000000.0) / m_ui64freq);
		} else {
			return (UINT64)(GetTickCount() - (DWORD)m_ui64BeginCount) * 1000;
		}
	}

	void Reset()
	{
		if (m_bFlag)       //if(High resolution timer is supported
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&m_ui64BeginCount);
		}
		else
		{
			m_ui64BeginCount = GetTickCount();
		}
	}

private:
	UINT64 m_ui64BeginCount;
	UINT64 m_ui64Elapse, m_ui64freq;
	BOOL m_bFlag;
};