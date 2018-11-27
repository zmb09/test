
// testMvMatroxDeviceDlg.h : header file
//

#pragma once

#include "mvMatroxDevice.h"
#include "CmvMatroxChannel.h"
#include "CmvMatroxDevice.h"
#include "CmvMatroxDeviceManager.h"
// CtestMvMatroxDeviceDlg dialog
class CtestMvMatroxDeviceDlg : public CDialogEx
{
// Construction
public:
	CtestMvMatroxDeviceDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTMVMATROXDEVICE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	HRESULT CtestMvMatroxDeviceDlg::_DefineAudioSamplesDescription(SMvCreateAudioSamplesDescription * io_psAudioSamplesDescription);
	SMvResolutionInfo m_sResolution;
	EMvChannelType m_eAudioChannelType;

	CmvMatroxDeviceManager m_CmvMatroxDeviceManager;
	TMvSmartPtr<IMvSurfacePool>           m_pJBypassInputVideoPool;
	TMvSmartPtr<IMvAudioSamplesPool>                m_pJAudioSamplesPoolForPlayback;

	TMvSmartPtr<CmvMatroxChannel> m_pJNsdOutputDeviceControl;

	CMvThread m_oThread;
	HANDLE m_hEvent;
	bool m_bplay;

	static UINT __stdcall ThreadRun(void* pThis);
	HRESULT ThreadLoop();

	HRESULT WriteAudioBuffer(IMvAudioSamples * pJNsAudioSamples);
	//HRESULT WriteVedioBuffer(IMvSurface* in_pNsSurface);
	HRESULT WriteVedioBuffer1(IMvSurface* in_pNsSurface);
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonSeek();
	afx_msg void OnBnClickedButtonplay();
	afx_msg void OnClose();
};
