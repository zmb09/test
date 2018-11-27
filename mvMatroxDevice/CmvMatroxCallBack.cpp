#include "StdAfx.h"
#include "CmvMatroxCallBack.h"


CSystemTopologyErrorCallback::CSystemTopologyErrorCallback()
	: CMvUnknown(L"CSystemTopologyErrorCallback", NULL)
{
}

CSystemTopologyErrorCallback::~CSystemTopologyErrorCallback(void)
{
}

HRESULT CSystemTopologyErrorCallback::NotifyAsyncError(const SMvAsyncErrorInformation& in_krsAsynErrorInfo)
{
	return MV_NOERROR;
}

//// ----------------------------------------------------------------------------
//void CSystemTopologyErrorCallback::SetResolution(SMvResolutionInfo in_sResolution)
//{
//	m_sResolution.size =sizeof(SMvResolutionInfo);
//	m_sResolution = in_sResolution;
//}





CErrorNotificationCallback::CErrorNotificationCallback()
	: CMvUnknown(L"CErrorNotificationCallback", NULL)
{
}

CErrorNotificationCallback::~CErrorNotificationCallback(void)
{
}

HRESULT CErrorNotificationCallback::Notify(HRESULT in_hrError)
{
	//CString strError = _T("CErrorNotificationCallback::Notify: An asychronous error was reported.");
	//CString strMessage;
	//strMessage.Format(_T("%s\nError 0x%X\n<%ls>"), strError, in_hrError, mvErrorToString(in_hrError));

	//CmvsCGDemoTPDlg dlg;
	//dlg.ShowDialogOnError(strMessage);

	return MV_NOERROR;
}



CGenlockStatusCallback::CGenlockStatusCallback()
	: CMvUnknown(L"CGenlockStatusCallback", NULL)
{
	ZeroMemory(&m_sGenlockStatus, sizeof(m_sGenlockStatus));
}

CGenlockStatusCallback::~CGenlockStatusCallback(void)
{
}

HRESULT CGenlockStatusCallback::Notify(SMvGenlockInputStatus& in_rsGenlockStatus)
{

	//Update genlock status
	m_sGenlockStatus = in_rsGenlockStatus;

	//// Check the genlock status
	//CString strGenlockStatusNotify;
	//strGenlockStatusNotify;

	//if (in_rsGenlockStatus.eGenlockState == keGenlockStateLocked)
	//{
	//	strGenlockStatusNotify.Format(_T("Genlock status: locked......."));
	//}
	//else
	//{
	//	strGenlockStatusNotify.Format(_T("Genlock status: unlocked......."));
	//}

	//CmvsCGDemoTPDlg dlg;
	//dlg.ShowDialogOnError(strGenlockStatusNotify);

	return MV_NOERROR;
}

// ---------------------------------------------------------------------------------------------------------------------
void CGenlockStatusCallback::GetStatus(SMvGenlockInputStatus& io_rsGenlockStatus)
{
	CMvAutoLock oLock(&m_csStatusLock);
	io_rsGenlockStatus = m_sGenlockStatus;
}


