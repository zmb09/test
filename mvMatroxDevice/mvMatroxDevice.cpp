#include "StdAfx.h"
//==========================================================================;
//
// (c) Copyright NewAuto Silicon Valley Video Technology CO.,Ltd., 2005. All rights reserved. 
//
// This code and information is provided "as is" without warranty of any kind, 
// either expressed or implied, including but not limited to the implied 
// warranties of merchantability and/or fitness for a particular purpose.
//
//--------------------------------------------------------------------------;
//	nsdDv1394.cpp
//----------------------------------------------------------------
//   Birth Date:       Aug 29. 2005
//   Operating System: WinXP
//   Author:           NewAuto NLE Team
//----------------------------------------------------------------
//   
//   Beijing China. 
//----------------------------------------------------------------

// nsdDeckLinkIODevice.cpp : Defines the entry point for the DLL application.
//
//#include "CNsdDSXLEIODeviceAccess.h"
//#include "CNsdDSXLEMultipleIODeviceAccess.h"
//#include "CNsdMultipleChannelDSXLEIODeviceAccess.h"
#include "CmvMatroxDeviceManager.h"
//#include "nsFactoryTemplate.h"
//////////////////////////////////////////////////////////////////////////

static const GUID CLSID_mvMatroxDevice = 
{ 0xb87a1a21, 0x9466, 0x4dce, { 0x80, 0x10, 0xb5, 0x2, 0x25, 0x7e, 0x40, 0x7b } };


void _mvStartWPP()
{
}

void _mvStopWPP()
{
}

CMvFactoryTemplate g_aoTemplates[] = 
{
	//{
	//	L"NewAuto DSXLE IO module",
	//	&CLSID_NsdDSXLEInputOutputAccess,
	//	CNsdDSXLEIODeviceAccess::CreateInstance,
	//	CNsdDSXLEIODeviceAccess::DllInit,
	//	NULL
	//},
	//{
	//   L"NewAuto DSXLE MIO module",
	//	&CLSID_NsdDSXMultipleInputOutputAccess,
	//	CNsdDSXLEMultipleIODeviceAccess::CreateInstance,
	//	CNsdDSXLEMultipleIODeviceAccess::DllInit,
	//	NULL
	//},
 //   {
	//	L"NewAuto DSXLE MultipleChannels IO module",
	//	    &CLSID_NsdMultipleChannelsDSXLEInputOutputAccess,
	//	    CNsdMultipleChannelDSXLEIODeviceAccess::CreateInstance,
	//		CNsdMultipleChannelDSXLEIODeviceAccess::DllInit,
	//		NULL
 //   },
	{
		L"NewAuto mvMatroxDevice",
			&CLSID_mvMatroxDevice,
			CmvMatroxDeviceManager::CreateInstance,
			CmvMatroxDeviceManager::DllInit,
			NULL
	}

};

int g_iTemplates = sizeof(g_aoTemplates) / sizeof(g_aoTemplates[0]);
