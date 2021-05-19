#ifndef PlayWnd_H
#define PlayWnd_H
//------------------------------------------------------------------------------
// File: PlayWnd.h
//
// Desc: DirectShow sample code - header file for video in window movie
//       player application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Function prototypes
//
HRESULT InitPlayerWindow(void);
HRESULT InitVideoWindow(int nMultiplier, int nDivider);
HRESULT HandleGraphEvent(void);
HRESULT StepOneFrame(void);
HRESULT StepFrames(int nFramesToStep);
HRESULT ModifyRate(double dRateAdjust);
HRESULT SetRate(double dRate);

BOOL GetFrameStepInterface(void);
BOOL GetClipFileName(LPTSTR szName);

void PaintAudioWindow(void);
void MoveVideoWindow(void);
void CheckVisibility(void);
void CloseInterfaces(void);

void OpenClip(void);
void PauseClip(void);
void StopClip(void);
void CloseClip(void);

void UpdateMainTitle(void);
//void CheckSizeMenu(WPARAM wParam);
//void EnablePlaybackMenu(BOOL bEnable, int nMediaType);
void GetFilename(TCHAR *pszFull, TCHAR *pszFile);
void Msg(TCHAR *szFormat, ...);

HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
void RemoveGraphFromRot(DWORD pdwRegister);

void InitVideo(HWND);
HRESULT PlayMovieInWindow(LPTSTR szFile);
HRESULT ToggleFullScreen(void);
BOOL IsWindowsMediaFile(LPTSTR lpszFile);
HRESULT ToggleMute(void);

extern bool SnapImage(IBasicVideo *mBasicVideo, TCHAR *szFilename) ;
//把Graph保存为gif文件
extern HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);
//pGB
//输入filter的名字，返回IBaseFilter指针
extern IBaseFilter *FindFilterFromName(LPTSTR szNameToFind);
extern void TToWide(LPTSTR szFile,WCHAR wFile[MAX_PATH]);
extern void WideToT(LPTSTR szFile,WCHAR wFile[MAX_PATH]);

HRESULT FindFilterInterface(
  IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
  REFGUID iid, // IID of the interface to retrieve.
  void **ppUnk) ;// Receives the interface pointer.

HRESULT FindPinInterface(
  IBaseFilter *pFilter, // Pointer to the filter to search.
  REFGUID iid, // IID of the interface.
  void **ppUnk); // Receives the interface pointer.

HRESULT FindInterfaceAnywhere(
  IGraphBuilder *pGraph,
  REFGUID iid,
  void **ppUnk);
 #include <streams.h> // Link to the DirectShow base class library
  // Define a typedef for a list of filters.
typedef CGenericList<IBaseFilter> CFilterList;
HRESULT GetNextFilter(
					  IBaseFilter *pFilter, // ??????filter
					  PIN_DIRECTION Dir, // ???÷??·??ò (upstream ???? downstream)
					  IBaseFilter **ppNext); // Receives a pointer to the next filter.

HRESULT GetPeerFilters(
					   IBaseFilter *pFilter, // Pointer to the starting filter
					   PIN_DIRECTION Dir, // Direction to search (upstream or downstream)
					   CFilterList &FilterList) ;// Collect the results in this list.


HRESULT RemoveAllFilters(IGraphBuilder *pGraph);

extern HRESULT AddFilterByCLSID(
  IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
  const GUID& clsid, // CLSID of the filter to create.
  LPCWSTR wszName, // A name for the filter.
  IBaseFilter **ppF); // Receives a pointer to the filter.

extern HRESULT GetUnconnectedPin(
						  IBaseFilter *pFilter, // Pointer to the filter.
						  PIN_DIRECTION PinDir, // Direction of the pin to find.
						  IPin **ppPin) ;// Receives a pointer to the pin.

extern HRESULT ConnectFilters(
					   IGraphBuilder *pGraph, // Filter Graph Manager.
					   IPin *pOut, // Output pin on the upstream filter.
					   IBaseFilter *pDest); // Downstream filter.


extern HRESULT ConnectFilters(
					   IGraphBuilder *pGraph,
					   IBaseFilter *pSrc,
					   IBaseFilter *pDest);

//显示属性页
extern void ShowProppage(IBaseFilter *pFilter,HWND hWnd) ;
//
// Constants
//
#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

// File filter for OpenFile dialog
#define FILE_FILTER_TEXT \
    TEXT("Video Files (*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v)\0*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v\0")\
    TEXT("Audio files (*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd)\0*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd\0")\
    TEXT("MIDI Files (*.mid, *.midi, *.rmi)\0*.mid; *.midi; *.rmi\0") \
    TEXT("Image Files (*.jpg, *.bmp, *.gif, *.tga)\0*.jpg; *.bmp; *.gif; *.tga\0") \
    TEXT("All Files (*.*)\0*.*;\0\0")

// Begin default media search at root directory
#define DEFAULT_MEDIA_PATH  TEXT("\\\0")

// Defaults used with audio-only files
#define DEFAULT_AUDIO_WIDTH     240
#define DEFAULT_AUDIO_HEIGHT    120
#define DEFAULT_VIDEO_WIDTH     320
#define DEFAULT_VIDEO_HEIGHT    240
#define MINIMUM_VIDEO_WIDTH     200
#define MINIMUM_VIDEO_HEIGHT    120

#define APPLICATIONNAME TEXT("PlayWnd Media Player\0")
#define CLASSNAME       TEXT("PlayWndMediaPlayer\0")

#define WM_GRAPHNOTIFY  WM_USER+13

enum PLAYSTATE {Stopped, Paused, Running, Init};

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return hr;}

#define LIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr);}

//
// Resource constants
//
#define IDI_PLAYWND                     100
//#define IDR_MENU                        101
//#define IDD_ABOUTBOX                    200
//#define ID_FILE_OPENCLIP                40001
//#define ID_FILE_EXIT                    40002
//#define ID_FILE_PAUSE                   40003
//#define ID_FILE_STOP                    40004
//#define ID_FILE_CLOSE                   40005
//#define ID_FILE_MUTE                    40006
//#define ID_FILE_FULLSCREEN              40007
//#define ID_FILE_SIZE_NORMAL             40008
//#define ID_FILE_SIZE_HALF               40009
//#define ID_FILE_SIZE_DOUBLE             40010
//#define ID_FILE_SIZE_QUARTER            40011
//#define ID_FILE_SIZE_THREEQUARTER       40012
//#define ID_HELP_ABOUT                   40014
#define ID_RATE_INCREASE                40020
#define ID_RATE_DECREASE                40021
#define ID_RATE_NORMAL                  40022
#define ID_RATE_DOUBLE                  40023
#define ID_RATE_HALF                    40024
#define ID_SINGLE_STEP                  40025


#define REGISTER_FILTERGRAPH

//
// Global data
//
extern HWND      ghApp;
extern HMENU     ghMenu;
extern HINSTANCE ghInst; 
extern TCHAR     g_szFileName[MAX_PATH];
extern BOOL      g_bAudioOnly, g_bFullscreen;
extern LONG      g_lVolume;
extern DWORD     g_dwGraphRegister;
extern PLAYSTATE g_psCurrent;
extern double    g_PlaybackRate;

// DirectShow interfaces
extern IGraphBuilder *pGB    ;
extern IMediaControl *pMC    ;
extern IMediaEventEx *pME    ;
extern IVideoWindow  *pVW    ;
extern IBasicAudio   *pBA    ;
extern IBasicVideo   *pBV    ;
extern IMediaSeeking *pMS    ;
extern IMediaPosition *pMP   ;
extern IVideoFrameStep *pFS  ;
extern const int AUDIO, VIDEO; // Used for enabling playback menu items

#endif