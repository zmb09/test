
// testMvMatroxDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "testMvMatroxDevice.h"
#include "testMvMatroxDeviceDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const unsigned long g_kulNsdAudioSamplesMaxBufferSize = 16016;
#define M_PI       3.14159265358979323846

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CtestMvMatroxDeviceDlg dialog

inline bool IsFieldBased(SMvResolutionInfo  &in_sResInfo)
{
	return in_sResInfo.eScanMode == keMvScanModeFirstFieldTop || in_sResInfo.eScanMode == keMvScanModeSecondFieldTop;
}


CtestMvMatroxDeviceDlg::CtestMvMatroxDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CtestMvMatroxDeviceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestMvMatroxDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CtestMvMatroxDeviceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CtestMvMatroxDeviceDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_Seek, &CtestMvMatroxDeviceDlg::OnBnClickedButtonSeek)
	ON_BN_CLICKED(IDC_BUTTON_play, &CtestMvMatroxDeviceDlg::OnBnClickedButtonplay)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CtestMvMatroxDeviceDlg message handlers

BOOL CtestMvMatroxDeviceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL); 
	m_bplay = false;

	m_eAudioChannelType = keChannelTypeStereo;

	//m_sResolution = {sizeof(SMvResolutionInfo)};

	m_sResolution.size = sizeof(SMvResolutionInfo);
	m_sResolution.eAspectRatio = keMvAspectRatio_16_9;
	m_sResolution.eFrameRate = keMvFrameRate50;
	m_sResolution.eScanMode = keMvScanModeProgressive;
	m_sResolution.ulComponentBitCount = 10;
	m_sResolution.ulHeight = 2160;
	m_sResolution.ulWidth = 3840;

	HRESULT hr = MV_NOERROR;
	m_CmvMatroxDeviceManager.Init(&m_sResolution);
	mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture, 
		hr, "m_hsemaphoreClock = CreateSemaphore(NULL, 0, 0xFF, NULL); error",true);

	int nDeviceCount = 0;
	hr = m_CmvMatroxDeviceManager.GetDeviceCount(&nDeviceCount);
	mv_SAFE_REPORT_ERROR( keLogPkgDirectShowDSXLEModule, keLogPkgDirectShowDSXLEModuleFuncCapture, 
		hr, "m_hsemaphoreClock = CreateSemaphore(NULL, 0, 0xFF, NULL); error",true);

	ASSERT(nDeviceCount >=1);

	hr = m_CmvMatroxDeviceManager.GetOutputDeviceByIndex(&m_pJNsdOutputDeviceControl,0,0);
	mv_SAFE_REPORT_ERROR( 0, 0,hr,
		"CNsdGraphStateManager::SetEditResolution",true);

	hr = m_pJNsdOutputDeviceControl->Open(&m_sResolution);
	mv_SAFE_REPORT_ERROR( 0, 0,hr,
		"CNsdGraphStateManager::SetEditResolution",true);



	SMvCreateSurfaceDescription sSurfaceDesc;
	memset(&sSurfaceDesc, 0, sizeof(sSurfaceDesc));
	sSurfaceDesc.size = sizeof(sSurfaceDesc);
	sSurfaceDesc.ulWidth = m_sResolution.ulWidth;

	if (IsFieldBased(m_sResolution))
	{
		sSurfaceDesc.ulHeight = m_sResolution.ulHeight / 2;
		sSurfaceDesc.ePolarity = keMvPolarityFirstField;
	}
	else
	{
		sSurfaceDesc.ulHeight = m_sResolution.ulHeight;
		sSurfaceDesc.ePolarity = keMvPolarityProgressiveFrame;
	}

	sSurfaceDesc.ulDepth = 1;
	sSurfaceDesc.ulComponentBitCount = 10;
	sSurfaceDesc.ulMipmapCount = 1;
	sSurfaceDesc.eFormat = keMvSurfaceFormatYUYV422;
	sSurfaceDesc.eType = keMvSurfaceType2D;
	sSurfaceDesc.eMemoryLocation = keMvMemoryLocationHost;
	sSurfaceDesc.eImageOrigin = keMvImageOriginTopLeft;
	sSurfaceDesc.eColorimetry = keMvColorimetryITUR_BT_709;


	hr = m_CmvMatroxDeviceManager.m_pJSystemConfiguration->CreateSurfacePool(L"Bypass input Video Pool",
		30,
		30,
		sSurfaceDesc,
		m_sResolution.eScanMode,
		false,
		&m_pJBypassInputVideoPool);
	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, 0,
		hr,	"CNsdMatroxChannel::SetClockInterface: m_oThread.Create Failed!", true);


	SMvCreateAudioSamplesDescription sAudioSamplesDescription;
	_DefineAudioSamplesDescription(&sAudioSamplesDescription);

	hr = m_CmvMatroxDeviceManager.m_pJSystemConfiguration->CreateAudioSamplesPool(L" m_pJAudioSamplesPoolForPlayback",
		100, 100, sAudioSamplesDescription, &m_pJAudioSamplesPoolForPlayback);
	mv_SAFE_REPORT_ERROR_RETURN(0,0,hr,
		"CmvMatroxDeviceManager::Init CoCreateInstance(CLSID_MvSystemConfiguration", true );


	try
	{
		m_oThread.Create(CtestMvMatroxDeviceDlg::ThreadRun, this);
	}
	catch (...)
	{
		mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDSXLEModule, 0,
			-1,	"CNsdMatroxChannel::SetClockInterface: m_oThread.Create Failed!", true);
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}


UINT CtestMvMatroxDeviceDlg::ThreadRun(void* pThis)
{
	CtestMvMatroxDeviceDlg* pThread = static_cast<CtestMvMatroxDeviceDlg*>(pThis);
	pThread->ThreadLoop();
	return 0;
}

HRESULT CtestMvMatroxDeviceDlg::WriteVedioBuffer1(IMvSurface* in_pNsSurface)
{
	HRESULT hr = MV_NOERROR;
	SMvLockSurfaceDescription sLockSurface = { sizeof( SMvLockSurfaceDescription )};		
	hr = in_pNsSurface->Lock(0, keMvFaceTypeFront, &sLockSurface);
	mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

	static int count = 0;
	count++;
	uint8_t * pSrc  = (uint8_t *)sLockSurface.pBuffer;
	int nPitch = sLockSurface.ulRowPitchInBytes;

	if (count%2 == 0)
	{
		memset(pSrc,0,nPitch*sLockSurface.ulHeight);
	}
	else
	{
		memset(pSrc,128,nPitch*sLockSurface.ulHeight);
	}


	//int nHeight = sLockSurface.ulHeight*3/4;
	//int nWidth = sLockSurface.ulWidth*3/4;
	//for (int y = sLockSurface.ulHeight/4; y < nHeight; y++) {
	//	for (int x = sLockSurface.ulWidth*2/4; x < nWidth; x+=4) {
	//		//frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
	//		pSrc[y*nPitch+x] = x + y + count * 3;
	//		pSrc[y*nPitch+x+2] = x + 2 + y + count * 3;

	//		pSrc[y*nPitch+x+1] = 128 + y + count * 2;
	//		pSrc[y*nPitch+x+3] = 64 + x +3+ count * 5;
	//	}
	//}

	hr = in_pNsSurface->Unlock(0, keMvFaceTypeFront);
	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

	return hr;
}

HRESULT CtestMvMatroxDeviceDlg::WriteAudioBuffer(IMvAudioSamples * pJNsAudioSamples)
{
	HRESULT hr = MV_NOERROR;

	unsigned char* pSrcBuf = NULL;
	unsigned long ulSrclength;
	hr =  pJNsAudioSamples->GetBufferAndLength((void**)&pSrcBuf,&ulSrclength);
	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

	//ASSERT(ulDstlength>=ulSrclength);

	SMvAudioSamplesDescription sDescSrcAudio;
	sDescSrcAudio.size = sizeof(SMvAudioSamplesDescription);
	hr = pJNsAudioSamples->GetAudioSamplesDescription(&sDescSrcAudio);
	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

	unsigned long ulSampleCount = 0;
	hr = pJNsAudioSamples->GetValidBufferLengthInSamples(&ulSampleCount);
	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);


	ulSrclength = 960*4;

	hr = pJNsAudioSamples->SetLength(ulSrclength);
	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsAudioSamples->GetValidBufferLengthInSamples(&ulSampleCount);
	mv_SAFE_REPORT_ERROR(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
		hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);



	/* encode a single tone sound */
	double t = 0;
	double tincr = 2 * M_PI * 880.0 / sDescSrcAudio.sWaveFormat.ulSamplesPerSec*2;

	uint32_t *samples = (uint32_t*)pSrcBuf;

	for (int j = 0; j < ulSampleCount; j++) 
	{
		samples[2*j] = (int)(sin(t) * 10000)<<16;

		for (int k = 1; k < sDescSrcAudio.sWaveFormat.eChannelType; k++)
			samples[2*j + k] = samples[2*j];
		t += tincr;
	}


	return hr;
}


HRESULT CtestMvMatroxDeviceDlg::ThreadLoop()
{
	HRESULT hr = MV_NOERROR;

	hr = ::CoInitialize(0);
	mv_SAFE_REPORT_ERROR( 0, 0,hr,
		"CNsdGraphStateManager::SetEditResolution",true);

	bool bContinue = true;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	//__int64 ui64CurrentTime = 0;
	//hr = m_pJNsdOutputDeviceControl->GetTime(&ui64CurrentTime);
	//mv_SAFE_REPORT_ERROR_RETURN( 0, 0,hr,
	//	"CNsdGraphStateManager::SetEditResolution",true);
	//uint64_t ui64TimeInFrame = MvGetSampleForNanoTime( ui64CurrentTime,m_sResolution ) +8;

	uint64_t ui64TimeInFrame = 0;

	while(bContinue)
	{
		if(!m_bplay)
		{
			WaitForSingleObject(m_hEvent, INFINITE);
		}

		ui64TimeInFrame++;
		uint64_t ui64TimeStamp = MvGetNanoTimeForSample( ui64TimeInFrame , m_sResolution );
		{
			__int64 ui64CurrentTime = 0;
			hr = m_pJNsdOutputDeviceControl->GetTime(&ui64CurrentTime);
			mv_SAFE_REPORT_ERROR_RETURN( 0, 0,hr,"CNsdGraphStateManager::SetEditResolution",true);

			uint64_t ui64TimeInFrameCurrent = MvGetSampleForNanoTime( ui64CurrentTime,m_sResolution ) +4;

			if(ui64TimeInFrame<ui64TimeInFrameCurrent)
			{
				ui64TimeStamp = MvGetNanoTimeForSample( ui64TimeInFrameCurrent , m_sResolution );
				ui64TimeInFrame = ui64TimeInFrameCurrent;
			}

			{
				static int count=0;
				++count;
				if(count<50 || !m_bplay )
				{
					char temp[400];
					sprintf_s(temp,"f_y,CtestMvMatroxDeviceDlg::ThreadLoop,ui64TimeStamp(%I64dms) - ui64CurrentTime(%I64dms)=%I64dms,count=%d",
						ui64TimeStamp/10000, ui64CurrentTime/10000,(ui64TimeStamp-ui64CurrentTime)/10000,count);
					OutputDebugString(temp);
				}
			}
		}

		

		TMvSmartPtr<IMvSurface> pJNsSurface;
		TMvSmartPtr<IMvAudioSamples> pJNsAudioSamples;
		TMvSmartPtr < IMvAVContent > pJNsAVContent;

		//hr = m_pJResourceManager->GetSurface(m_sInputSurPoolDesc,&pJNsSurface,INFINITE);
		hr = m_pJBypassInputVideoPool->WaitForSurface(INFINITE,&pJNsSurface);
		mv_SAFE_REPORT_ERROR_RETURN( 0, 0,hr,
			"CNsdGraphStateManager::SetEditResolution",true);

		hr = pJNsSurface->QueryInterface(IID_IMvAVContent, (void**)&pJNsAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsAVContent->IncrementReadCount();
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsAVContent->SetTimeStampInfo(&ui64TimeStamp);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		pJNsAVContent = NULL;


		//hr = m_pJResourceManager->GetAudioSamples(m_sInputSampleDesc,&pJNsAudioSamples,INFINITE);
		hr = m_pJAudioSamplesPoolForPlayback->WaitForAudioSamples(INFINITE,&pJNsAudioSamples);
		mv_SAFE_REPORT_ERROR_RETURN( 0, 0,hr,
			"CNsdGraphStateManager::SetEditResolution",true);

		hr = pJNsAudioSamples->QueryInterface(IID_IMvAVContent, (void**)&pJNsAVContent);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsAVContent->IncrementReadCount();
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		hr = pJNsAVContent->SetTimeStampInfo(&ui64TimeStamp);
		mv_SAFE_REPORT_ERROR_RETURN(keLogPkgDirectShowDeckLinkModule, keLogPkgDirectShowDeckLinkModuleFuncPlayBackDevice,
			hr, "CNsdStreamBasedOutputDevice::PlayVideoBuffer: SignalReadCompletion failed!", true);

		pJNsAVContent = NULL;

		WriteAudioBuffer(pJNsAudioSamples);
		hr = m_pJNsdOutputDeviceControl->PlayAudioBuffer(0,pJNsAudioSamples);
		mv_SAFE_REPORT_ERROR_RETURN( 0, 0,hr,
			"CNsdGraphStateManager::SetEditResolution",true);

		WriteVedioBuffer1(pJNsSurface);

		hr = m_pJNsdOutputDeviceControl->PlayVideoBuffer(pJNsSurface);
		mv_SAFE_REPORT_ERROR_RETURN( 0, 0,hr,
			"CNsdGraphStateManager::SetEditResolution",true);

	}

	return hr;
}
void CtestMvMatroxDeviceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CtestMvMatroxDeviceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CtestMvMatroxDeviceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HRESULT CtestMvMatroxDeviceDlg::_DefineAudioSamplesDescription(SMvCreateAudioSamplesDescription * io_psAudioSamplesDescription)
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

void CtestMvMatroxDeviceDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	exit(0);
	CDialogEx::OnOK();
}


void CtestMvMatroxDeviceDlg::OnBnClickedButtonSeek()
{
	// TODO: Add your control notification handler code here
	m_bplay = false;
	SetEvent(m_hEvent);
}


void CtestMvMatroxDeviceDlg::OnBnClickedButtonplay()
{
	// TODO: Add your control notification handler code here
	m_bplay = true;
	SetEvent(m_hEvent);
	
}


void CtestMvMatroxDeviceDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	exit(0);
	CDialogEx::OnClose();
}
