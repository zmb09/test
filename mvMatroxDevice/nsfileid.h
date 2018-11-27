
/* $NoKeywords: $ */



#ifndef _NSFILEID_H_
#define _NSFILEID_H_


 
#ifdef _WIN32
  #include <winver.h>
#else
  #include <ver.h>
#endif



//////////////////////////////////////////////////////////////////////////////
//
// This is the file version number
//
#define FILE_VERSION_RES_MAJOR_VER 1
#define FILE_VERSION_RES_MINOR_VER 0
#define FILE_VERSION_RES_MAJOR_BUILD 0
#define FILE_VERSION_RES_BUILD 203
#define FILE_VERSION_RES_BUILD_STR "203"
//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
// The following defines are required on a file-by-file basis
//
#define FILE_COMMENT                      "Newauto Video \0"
#define FILE_VERSION_RES_BIN_NAME         "nsdDSXLEIODevice.dll\0"
#define FILE_VERSION_RES_BIN_DESCRIPTION  "New Project module\0"
//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
// If there is a special build, put the reason here and exchange the comments
//
//#define VER_SPECIALBUILD   VS_FF_SPECIALBUILD
#define VER_SPECIALBUILD   0
#define VERSION_RES_SPECIALBUILD_STRING "\0"
//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
// Uncomment one of the following lines...
//
// #define VERSION_RES_TYPE        VFT_UNKNOWN   
// #define VERSION_RES_TYPE        VFT_APP       
#define VERSION_RES_TYPE        VFT_DLL
// #define VERSION_RES_TYPE        VFT_DRV       
// #define VERSION_RES_TYPE        VFT_FONT      
// #define VERSION_RES_TYPE        VFT_VXD       
// #define VERSION_RES_TYPE        VFT_STATIC_LIB
//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
// Uncomment one of the following lines...
//
// #define VERSION_RES_SUBTYPE     VFT2_UNKNOWN
// #define VERSION_RES_SUBTYPE     VFT2_DRV_PRINTER    
// #define VERSION_RES_SUBTYPE     VFT2_DRV_KEYBOARD   
// #define VERSION_RES_SUBTYPE     VFT2_DRV_LANGUAGE   
// #define VERSION_RES_SUBTYPE     VFT2_DRV_DISPLAY    
// #define VERSION_RES_SUBTYPE     VFT2_DRV_MOUSE      
// #define VERSION_RES_SUBTYPE     VFT2_DRV_NETWORK    
#define VERSION_RES_SUBTYPE     VFT2_DRV_SYSTEM     
// #define VERSION_RES_SUBTYPE     VFT2_DRV_INSTALLABLE
// #define VERSION_RES_SUBTYPE     VFT2_DRV_SOUND      
// #define VERSION_RES_SUBTYPE     VFT2_DRV_COMM       
// #define VERSION_RES_SUBTYPE     VFT2_DRV_INPUTMETHOD
// 
//////////////////////////////////////////////////////////////////////////////


#endif // _NSFILEID_H_

