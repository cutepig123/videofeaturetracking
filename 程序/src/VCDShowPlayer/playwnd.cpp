//------------------------------------------------------------------------------
// File: PlayWnd.cpp
//
// Desc: DirectShow sample code - a simple audio/video media file player
//       application.  Pause, stop, mute, and fullscreen mode toggle can
//       be performed via keyboard commands.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <tchar.h>
#include <atlbase.h>

#include "playwnd.h"

// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
HWND      ghApp=0;
HMENU     ghMenu=0;
HINSTANCE ghInst=0;
TCHAR     g_szFileName[MAX_PATH]={0};
BOOL      g_bAudioOnly=FALSE, g_bFullscreen=FALSE;
LONG      g_lVolume=VOLUME_FULL;
DWORD     g_dwGraphRegister=0;
PLAYSTATE g_psCurrent=Stopped;
double    g_PlaybackRate=1.0;

// DirectShow interfaces
IGraphBuilder *pGB   = NULL;
IMediaControl *pMC   = NULL;
IMediaEventEx *pME   = NULL;
IVideoWindow  *pVW   = NULL;
IBasicAudio   *pBA   = NULL;
IBasicVideo   *pBV   = NULL;
IMediaSeeking *pMS   = NULL;
IMediaPosition *pMP  = NULL;
IVideoFrameStep *pFS = NULL;

ICaptureGraphBuilder2 * pCapture = NULL;
PLAYSTATE psCurrent = Stopped;

const int AUDIO=1, VIDEO=2; // Used for enabling playback menu items

//以上的全局变量一个不能少
//在窗口中打开文件播放
HRESULT PlayMovieInWindow(LPTSTR szFile)
{
    USES_CONVERSION;
    WCHAR wFile[MAX_PATH];
    HRESULT hr;

    if (!szFile)
        return E_POINTER;

    // Clear open dialog remnants before calling RenderFile()
    UpdateWindow(ghApp);

    // Convert filename to wide character string
    wcsncpy(wFile, T2W(szFile), NUMELMS(wFile)-1);
    wFile[MAX_PATH-1] = 0;

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&pGB));

    if(SUCCEEDED(hr))
    {
        // Have the graph builder construct its the appropriate graph automatically
        JIF(pGB->RenderFile(wFile, NULL));

        // QueryInterface for DirectShow interfaces
        JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        JIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
        JIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));
        JIF(pGB->QueryInterface(IID_IMediaPosition, (void **)&pMP));

        // Query for video interfaces, which may not be relevant for audio files
        JIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));
        JIF(pGB->QueryInterface(IID_IBasicVideo, (void **)&pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        JIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

        // Is this an audio-only file (no video component)?
        CheckVisibility();

        // Have the graph signal event via window callbacks for performance
		//#define WM_GRAPHNOTIFY WM_USER+13
        JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

        if (!g_bAudioOnly)
        {
            // Setup the video window
            JIF(pVW->put_Owner((OAHWND)ghApp));
            JIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));

            JIF(InitVideoWindow(1, 1));
            GetFrameStepInterface();
        }
        else
        {
            // Initialize the default player size and enable playback menu items
            JIF(InitPlayerWindow());
//            //EnablePlaybackMenu(TRUE, AUDIO);
        }

        // Complete window initialization
//        CheckSizeMenu(ID_FILE_SIZE_NORMAL);
        ShowWindow(ghApp, SW_SHOWNORMAL);
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        g_bFullscreen = FALSE;
        g_PlaybackRate = 1.0;
        UpdateMainTitle();

#ifdef REGISTER_FILTERGRAPH
        hr = AddGraphToRot(pGB, &g_dwGraphRegister);
        if (FAILED(hr))
        {
            Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
            g_dwGraphRegister = 0;
        }
#endif

        // Run the graph to play the media file
        JIF(pMC->Run());

        g_psCurrent=Running;
        SetFocus(ghApp);
    }

    return hr;
}

//pBV
//VIDEO
//HWND ghApp
//初始化窗口大小
HRESULT InitVideoWindow(int nMultiplier, int nDivider)
{
    LONG lHeight, lWidth;
    HRESULT hr = S_OK;
    RECT rect;

    if (!pBV)
        return S_OK;

    // Read the default video size
    hr = pBV->GetVideoSize(&lWidth, &lHeight);
    if (hr == E_NOINTERFACE)
        return S_OK;

//    //EnablePlaybackMenu(TRUE, VIDEO);

    // Account for requests of normal, half, or double size
    lWidth  = lWidth  * nMultiplier / nDivider;
    lHeight = lHeight * nMultiplier / nDivider;

    int nTitleHeight  = GetSystemMetrics(SM_CYCAPTION);
    int nBorderWidth  = GetSystemMetrics(SM_CXBORDER);
    int nBorderHeight = GetSystemMetrics(SM_CYBORDER);

    // Account for size of title bar and borders for exact match
    // of window client area to default video size
    SetWindowPos(ghApp, NULL, 0, 0, lWidth + 2*nBorderWidth,
            lHeight + nTitleHeight + 2*nBorderHeight,
            SWP_NOMOVE | SWP_NOOWNERZORDER);

    GetClientRect(ghApp, &rect);
    JIF(pVW->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom));

    return hr;
}


HRESULT InitPlayerWindow(void)
{
    // Reset to a default size for audio and after closing a clip
    SetWindowPos(ghApp, NULL, 0, 0,
                 DEFAULT_AUDIO_WIDTH,
                 DEFAULT_AUDIO_HEIGHT,
                 SWP_NOMOVE | SWP_NOOWNERZORDER);

    // Check the 'full size' menu item
//    CheckSizeMenu(ID_FILE_SIZE_NORMAL);
    ////EnablePlaybackMenu(FALSE, 0);

    return S_OK;
}

//pVW，ghApp
//使视频窗口和程序窗口一样大
void MoveVideoWindow(void)
{
    HRESULT hr;

    // Track the movement of the container window and resize as needed
    if(pVW)
    {
        RECT client;

        GetClientRect(ghApp, &client);
        hr = pVW->SetWindowPosition(client.left, client.top,
                                    client.right, client.bottom);
    }
}


void CheckVisibility(void)
{
    long lVisible;
    HRESULT hr;

    if ((!pVW) || (!pBV))
    {
        // Audio-only files have no video interfaces.  This might also
        // be a file whose video component uses an unknown video codec.
        g_bAudioOnly = TRUE;
        return;
    }
    else
    {
        // Clear the global flag
        g_bAudioOnly = FALSE;
    }

    hr = pVW->get_Visible(&lVisible);
    if (FAILED(hr))
    {
        // If this is an audio-only clip, get_Visible() won't work.
        //
        // Also, if this video is encoded with an unsupported codec,
        // we won't see any video, although the audio will work if it is
        // of a supported format.
        //
        if (hr == E_NOINTERFACE)
        {
            g_bAudioOnly = TRUE;
        }
        else
        {
            Msg(TEXT("Failed(%08lx) in pVW->get_Visible()!\r\n"), hr);
        }
    }
}


void PauseClip(void)
{
    if (!pMC)
        return;

    // Toggle play/pause behavior
    if((g_psCurrent == Paused) || (g_psCurrent == Stopped))
    {
        if (SUCCEEDED(pMC->Run()))
            g_psCurrent = Running;
    }
    else
    {
        if (SUCCEEDED(pMC->Pause()))
            g_psCurrent = Paused;
    }

    UpdateMainTitle();
}


void StopClip(void)
{
    HRESULT hr;

    if ((!pMC) || (!pMS))
        return;

    // Stop and reset postion to beginning
    if((g_psCurrent == Paused) || (g_psCurrent == Running))
    {
        LONGLONG pos = 0;
        hr = pMC->Stop();
        g_psCurrent = Stopped;

        // Seek to the beginning
        hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
            NULL, AM_SEEKING_NoPositioning);

        // Display the first frame to indicate the reset condition
        hr = pMC->Pause();
    }

    UpdateMainTitle();
}

//判断是否微软的多媒体文件
BOOL IsWindowsMediaFile(LPTSTR lpszFile)
{
    TCHAR szFilename[MAX_PATH];

    // Copy the file name to a local string and convert to lowercase
    _tcsncpy(szFilename, lpszFile, NUMELMS(szFilename)-1);
    szFilename[MAX_PATH-1] = 0;
    _tcslwr(szFilename);

    if (_tcsstr(szFilename, TEXT(".asf")) ||
        _tcsstr(szFilename, TEXT(".wma")) ||
        _tcsstr(szFilename, TEXT(".wmv")))
        return TRUE;
    else
        return FALSE;
}

//打开文件并播放，不用
void OpenClip()
{
    HRESULT hr;

    // If no filename specified by command line, show file open dialog
    if(g_szFileName[0] == L'\0')
    {
        TCHAR szFilename[MAX_PATH];

        UpdateMainTitle();

        // If no filename was specified on the command line, then our video
        // window has not been created or made visible.  Make our main window
        // visible and bring to the front to allow file selection.
        InitPlayerWindow();
        ShowWindow(ghApp, SW_SHOWNORMAL);
        SetForegroundWindow(ghApp);

        if (! GetClipFileName(szFilename))
        {
            DWORD dwDlgErr = CommDlgExtendedError();

            // Don't show output if user cancelled the selection (no dlg error)
            if (dwDlgErr)
            {
                Msg(TEXT("GetClipFileName Failed! Error=0x%x\r\n"), GetLastError());
            }
            return;
        }

        // This sample does not support playback of ASX playlists.
        // Since this could be confusing to a user, display a warning
        // message if an ASX file was opened.
        if (_tcsstr((_tcslwr(szFilename)), TEXT(".asx")))
        {
            Msg(TEXT("ASX Playlists are not supported by this application.\n\n")
                TEXT("Please select a valid media file.\0"));
            return;
        }

        // This sample does not offer support for Windows Media playback.
        // The PlayWndASF and Jukebox samples demonstrate how to add a 
        // Windows Media certificate handler to your app, along with giving
        // example code to load and connect the ASF Reader source filter.
        // To prevent problems with using the older (default) ASF reader here,
        // simply disable loading of ASF,WMA,WMV files.
        if (IsWindowsMediaFile(szFilename))
        {
            Msg(TEXT("Windows Media files (ASF,WMV,WMA) are not supported by this application.\n\n")
                TEXT("To play Windows Media files, use the PlayWndASF or Jukebox samples.\0"));
            return;
        }

        lstrcpy(g_szFileName, szFilename);
    }

    // Reset status variables
    g_psCurrent = Stopped;
    g_lVolume = VOLUME_FULL;

    // Start playing the media file
    hr = PlayMovieInWindow(g_szFileName);

    // If we couldn't play the clip, clean up
    if (FAILED(hr))
        CloseClip();
}

//ghApp
//跳出一个打开文件窗口，返回文件名字
BOOL GetClipFileName(LPTSTR szName)
{
    static OPENFILENAME ofn={0};
    static BOOL bSetInitialDir = FALSE;

    // Reset filename
    *szName = 0;

    // Fill in standard structure fields
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = ghApp;
    ofn.lpstrFilter       = FILE_FILTER_TEXT;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szName;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrTitle        = TEXT("Open Media File...\0");
    ofn.lpstrFileTitle    = NULL;
    ofn.lpstrDefExt       = TEXT("*\0");
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_PATHMUSTEXIST;

    // Remember the path of the first selected file
    if (bSetInitialDir == FALSE)
    {
        ofn.lpstrInitialDir = DEFAULT_MEDIA_PATH;
        bSetInitialDir = TRUE;
    }
    else
        ofn.lpstrInitialDir = NULL;

    // Create the standard file open dialog and return its result
    return GetOpenFileName((LPOPENFILENAME)&ofn);
}


void CloseClip()
{
    HRESULT hr;

    // Stop media playback
    if(pMC)
        hr = pMC->Stop();

    // Clear global flags
    g_psCurrent = Stopped;
    g_bAudioOnly = TRUE;
    g_bFullscreen = FALSE;

    // Free DirectShow interfaces
    CloseInterfaces();

    // Clear file name to allow selection of new file with open dialog
    g_szFileName[0] = L'\0';

    // No current media state
    g_psCurrent = Init;

    // Reset the player window
    RECT rect;
    GetClientRect(ghApp, &rect);
    InvalidateRect(ghApp, &rect, TRUE);

    UpdateMainTitle();
    InitPlayerWindow();
}

extern WCHAR g_wGrfFile[MAX_PATH];
//释放所有DSHow变量pVW  g_dwGraphRegister pME 。。。
void CloseInterfaces(void)
{
    HRESULT hr;

    // Relinquish ownership (IMPORTANT!) after hiding video window
    if(pVW)
    {
        hr = pVW->put_Visible(OAFALSE);
        hr = pVW->put_Owner(NULL);
    }

    // Disable event callbacks
    if (pME)
        hr = pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

#ifdef REGISTER_FILTERGRAPH
    if (g_dwGraphRegister)
    {
        RemoveGraphFromRot(g_dwGraphRegister);
        g_dwGraphRegister = 0;
    }
#endif

    // Release and zero DirectShow interfaces
    SAFE_RELEASE(pME);
    SAFE_RELEASE(pMS);
    SAFE_RELEASE(pMP);
    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pBA);
    SAFE_RELEASE(pBV);
    SAFE_RELEASE(pVW);
    SAFE_RELEASE(pFS);
    SAFE_RELEASE(pGB);

	SAFE_RELEASE(pCapture);

	memset(g_wGrfFile,0,MAX_PATH*sizeof(WCHAR));
}


//#ifdef REGISTER_FILTERGRAPH
//添加到Rot中，pdwRegister似乎是一个表示注册ID的变量
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) 
    {
        return E_FAIL;
    }

    WCHAR wsz[128];
    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
		/*[out] Pointer to a 32-bit value that can be used to identify this ROT entry in subsequent calls to IRunningObjectTable::Revoke or IRunningObjectTable::NoteChangeTime. The caller cannot specify NULL for this parameter. If an error occurs, *pdwRegister is set to zero. */
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}
//移除GraphFromRot
void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

//#endif

//跳出一个消息框
void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    // Display a message box with the formatted string
    MessageBox(NULL, szBuffer, TEXT("PlayWnd Sample"), MB_OK);
}


HRESULT ToggleMute(void)
{
    HRESULT hr=S_OK;

    if ((!pGB) || (!pBA))
        return S_OK;

    // Read current volume
    hr = pBA->get_Volume(&g_lVolume);
    if (hr == E_NOTIMPL)
    {
        // Fail quietly if this is a video-only media file
        return S_OK;
    }
    else if (FAILED(hr))
    {
        Msg(TEXT("Failed to read audio volume!  hr=0x%x\r\n"), hr);
        return hr;
    }

    // Switch volume levels
    if (g_lVolume == VOLUME_FULL)
        g_lVolume = VOLUME_SILENCE;
    else
        g_lVolume = VOLUME_FULL;

    // Set new volume
    JIF(pBA->put_Volume(g_lVolume));

    UpdateMainTitle();
    return hr;
}


void UpdateMainTitle(void)
{
    TCHAR szTitle[MAX_PATH]={0}, szFile[MAX_PATH]={0};

    // If no file is loaded, just show the application title
    if (g_szFileName[0] == L'\0')
    {
        _tcscpy(szTitle, APPLICATIONNAME);
    }

    // Otherwise, show useful information
    else
    {
        // Get file name without full path
        GetFilename(g_szFileName, szFile);

        char szPlaybackRate[16];
        if (g_PlaybackRate == 1.0)
            szPlaybackRate[0] = '\0';
        else
            sprintf(szPlaybackRate, "(Rate:%2.2f)\0", g_PlaybackRate);

        TCHAR szRate[20];
        USES_CONVERSION;
        _tcsncpy(szRate, A2T(szPlaybackRate), NUMELMS(szRate));

        // Update the window title to show filename and play state
        wsprintf(szTitle, TEXT("%s [%s] %s%s%s\0"),
                szFile,
                g_bAudioOnly ? TEXT("Audio\0") : TEXT("Video\0"),
                (g_lVolume == VOLUME_SILENCE) ? TEXT("(Muted)\0") : TEXT("\0"),
                (g_psCurrent == Paused) ? TEXT("(Paused)\0") : TEXT("\0"),
                szRate);
    }

    SetWindowText(ghApp, szTitle);
}

//输入全路经pszFull，返回文件名pszFile
void GetFilename(TCHAR *pszFull, TCHAR *pszFile)
{
    int nLength;
    TCHAR szPath[MAX_PATH]={0};
    BOOL bSetFilename=FALSE;

    // Strip path and return just the file's name
    _tcsncpy(szPath, pszFull, MAX_PATH);
    szPath[MAX_PATH-1] = 0;

    nLength = (int) _tcslen(szPath);

    for (int i=nLength-1; i>=0; i--)
    {
        if ((szPath[i] == '\\') || (szPath[i] == '/'))
        {
            szPath[i] = '\0';
            lstrcpyn(pszFile, &szPath[i+1], MAX_PATH);
            bSetFilename = TRUE;
            break;
        }
    }

    // If there was no path given (just a file name), then
    // just copy the full path to the target path.
    if (!bSetFilename)
        _tcsncpy(pszFile, pszFull, MAX_PATH);
        
    pszFile[MAX_PATH-1] = 0;        // Ensure null-termination
}

//g_bAudioOnly pVW g_bFullscreen ghApp
//在全屏和非全屏切换
HRESULT ToggleFullScreen(void)
{
    HRESULT hr=S_OK;
    LONG lMode;
    static HWND hDrain=0;

    // Don't bother with full-screen for audio-only files
    if ((g_bAudioOnly) || (!pVW))
        return S_OK;

    // Read current state
    JIF(pVW->get_FullScreenMode(&lMode));

    if (lMode == OAFALSE)
    {
        // Save current message drain
        LIF(pVW->get_MessageDrain((OAHWND *) &hDrain));

        // Set message drain to application main window
        LIF(pVW->put_MessageDrain((OAHWND) ghApp));

        // Switch to full-screen mode
        lMode = OATRUE;
        JIF(pVW->put_FullScreenMode(lMode));
        g_bFullscreen = TRUE;
    }
    else
    {
        // Switch back to windowed mode
        lMode = OAFALSE;
        JIF(pVW->put_FullScreenMode(lMode));

        // Undo change of message drain
        LIF(pVW->put_MessageDrain((OAHWND) hDrain));

        // Reset video window
        LIF(pVW->SetWindowForeground(-1));

        // Reclaim keyboard focus for player application
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        SetFocus(ghApp);
        g_bFullscreen = FALSE;
    }

    return hr;
}


//
// Some video renderers support stepping media frame by frame with the
// IVideoFrameStep interface.  See the interface documentation for more
// details on frame stepping.
//pGB
//检查是否支持步进，如果是就初始化pFS
BOOL GetFrameStepInterface(void)
{
    HRESULT hr;
    IVideoFrameStep *pFSTest = NULL;

    // Get the frame step interface, if supported
    hr = pGB->QueryInterface(__uuidof(IVideoFrameStep), (PVOID *)&pFSTest);
    if (FAILED(hr))
        return FALSE;

    // Check if this decoder can step
    hr = pFSTest->CanStep(0L, NULL);

    if (hr == S_OK)
    {
        pFS = pFSTest;  // Save interface to global variable for later use
        return TRUE;
    }
    else
    {
        pFSTest->Release();
        return FALSE;
    }
}

//pFS ；FILTER_STATE g_psCurrent
//步进 
HRESULT StepOneFrame(void)
{
    HRESULT hr=S_OK;

    // If the Frame Stepping interface exists, use it to step one frame
    if (pFS)
    {
        // The graph must be paused for frame stepping to work
        if (g_psCurrent != State_Paused)
            PauseClip();

        // Step the requested number of frames, if supported
        hr = pFS->Step(1, NULL);
    }

    return hr;
}

//pFS g_psCurrent PauseClip 
//步进多步
HRESULT StepFrames(int nFramesToStep)
{
    HRESULT hr=S_OK;

    // If the Frame Stepping interface exists, use it to step frames
    if (pFS)
    {
        // The renderer may not support frame stepping for more than one
        // frame at a time, so check for support.  S_OK indicates that the
        // renderer can step nFramesToStep successfully.
        if ((hr = pFS->CanStep(nFramesToStep, NULL)) == S_OK)
        {
            // The graph must be paused for frame stepping to work
            if (g_psCurrent != State_Paused)
                PauseClip();

            // Step the requested number of frames, if supported
            hr = pFS->Step(nFramesToStep, NULL);
        }
    }

    return hr;
}

//pMP UpdateMainTitle（）
//修改播放速度
HRESULT ModifyRate(double dRateAdjust)
{
    HRESULT hr=S_OK;
    double dRate;

    // If the IMediaPosition interface exists, use it to set rate
    if ((pMP) && (dRateAdjust != 0))
    {
        if ((hr = pMP->get_Rate(&dRate)) == S_OK)
        {
            // Add current rate to adjustment value
            double dNewRate = dRate + dRateAdjust;
            hr = pMP->put_Rate(dNewRate);

            // Save global rate
            if (SUCCEEDED(hr))
            {
                g_PlaybackRate = dNewRate;
                UpdateMainTitle();
            }
        }
    }

    return hr;
}


HRESULT SetRate(double dRate)
{
    HRESULT hr=S_OK;

    // If the IMediaPosition interface exists, use it to set rate
    if (pMP)
    {
        hr = pMP->put_Rate(dRate);

        // Save global rate
        if (SUCCEEDED(hr))
        {
            g_PlaybackRate = dRate;
            UpdateMainTitle();
        }
    }

    return hr;
}

//pME pMS 
//处理Graph的各种事件WM_GRAPHNOTIFY
HRESULT HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    // Make sure that we don't access the media event interface
    // after it has already been released.
    if (!pME)
        return S_OK;

    // Process all queued events
    while(SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR *) &evParam1,
                    (LONG_PTR *) &evParam2, 0)))
    {
        // Free memory associated with callback, since we're not using it
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);

        // If this is the end of the clip, reset to beginning
        if(EC_COMPLETE == evCode)
        {
            LONGLONG pos=0;

            // Reset to first frame of movie
            hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                   NULL, AM_SEEKING_NoPositioning);
            if (FAILED(hr))
            {
                // Some custom filters (like the Windows CE MIDI filter)
                // may not implement seeking interfaces (IMediaSeeking)
                // to allow seeking to the start.  In that case, just stop
                // and restart for the same effect.  This should not be
                // necessary in most cases.
                if (FAILED(hr = pMC->Stop()))
                {
                    Msg(TEXT("Failed(0x%08lx) to stop media clip!\r\n"), hr);
                    break;
                }

                if (FAILED(hr = pMC->Run()))
                {
                    Msg(TEXT("Failed(0x%08lx) to reset media clip!\r\n"), hr);
                    break;
                }
            }
        }
    }

    return hr;
}

//ghApp
void InitVideo(HWND hWnd)
{
    USES_CONVERSION;

    // Initialize COM
    //if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	if(FAILED(CoInitialize(NULL)))
    {
        Msg(TEXT("CoInitialize Failed!\r\n"));
        exit(1);
    }
	
	//ghApp=AfxGetMainWnd()->m_hWnd;
	ghApp=hWnd;// ::GetDlgItem(AfxGetMainWnd()->m_hWnd,IDM_PIC);

	//PlayMovieInWindow("f:\\花香.DAT.mpg");
    // Was a filename specified on the command line?
//    if(lpCmdLine[0] != '\0')
//    {
//        USES_CONVERSION;
//        _tcsncpy(g_szFileName, A2T(lpCmdLine), NUMELMS(g_szFileName));        
//    }
//    g_szFileName[MAX_PATH-1] = 0;       // Ensure null-termination    

    // Set initial media state
    g_psCurrent = Init;

    // Finished with COM
   // CoUninitialize();

    
}



bool SnapImage(IBasicVideo *mBasicVideo, TCHAR *szFilename) 
{ 
	if (mBasicVideo) 
	{ 
		long bitmapSize = 0; 
		if(SUCCEEDED(mBasicVideo->GetCurrentImage(&bitmapSize, 0))) 
		{ 
			//if语句里面的操作时取得buffer的size。
			//当我们在布确定image buffer的大小的情况下，我们给
			//GetCurrentImage的第二个参数传递0或者NULL，取得buffer的
			//大小供以后使用。
			bool pass = false; 
			unsigned char * buffer = new unsigned char[bitmapSize]; 
			if(SUCCEEDED(mBasicVideo->GetCurrentImage(&bitmapSize,(long*)buffer))) 
			{ 
				//此时已经用到刚才所取得的大小（分配空间）
				BITMAPFILEHEADER  hdr;    //Bitmap的头信息
				LPBITMAPINFOHEADER   lpbi; // Bitmap的文件信息（包括数据）
				
				lpbi = (LPBITMAPINFOHEADER)buffer; 
				
				int nColors = 1 << lpbi->biBitCount; 
				if (nColors > 256) 
					nColors = 0; 
				
				hdr.bfType    = ((WORD) ('M' << 8) | 'B');    //always is "BM"
				hdr.bfSize    = bitmapSize + sizeof( hdr ); 
				hdr.bfReserved1   = 0; 
				hdr.bfReserved2   = 0; 
				hdr.bfOffBits     = (DWORD) (sizeof(BITMAPFILEHEADER) + lpbi->biSize );
				//CFile bitmapFile(outFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary); 
				//bitmapFile.Write(&hdr, sizeof(BITMAPFILEHEADER)); 
				//bitmapFile.Write(buffer, bitmapSize); 
				//bitmapFile.Close(); 
				// Create a new file to store the bitmap data
            HANDLE hFile = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE)
                return FALSE;
			 // Write the bitmap header and bitmap bits to the file
			DWORD dwWritten=sizeof(BITMAPFILEHEADER);
            WriteFile(hFile, (LPCVOID) &hdr, sizeof(BITMAPFILEHEADER), &dwWritten, 0);
            WriteFile(hFile, (LPCVOID) buffer, bitmapSize, &dwWritten, 0);

            // Close the file
            CloseHandle(hFile);

				pass = true; 
			} 
			delete [] buffer; //数据用过之后记得要释放空间
			return true; 
		} 
	} 
	
	return false; 
} 

//pGB
//输入filter的名字，返回IBaseFilter指针
IBaseFilter *FindFilterFromName(LPTSTR szNameToFind)
{
    USES_CONVERSION;

    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;
    BOOL bFound = FALSE;

	if(!pGB)return 0;
    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
        return NULL;

    // Enumerate all filters in the graph
    while((pEnum->Next(1, &pFilter, &cFetched) == S_OK) && (!bFound))
    {
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
        
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (FAILED(hr))
        {
            pFilter->Release();
            pEnum->Release();
            return NULL;
        }

        // Compare this filter's name with the one we want
        lstrcpy(szName, W2T(FilterInfo.achName));
        if (! lstrcmp(szName, szNameToFind))
        {
            bFound = TRUE;
        }

        FilterInfo.pGraph->Release();

        // If we found the right filter, don't release its interface.
        // The caller will use it and release it later.
        if (!bFound)
            pFilter->Release();
        else
            break;
    }
    pEnum->Release();

    return (bFound ? pFilter : NULL);
}

//hr = EnumPins(pFilter, PINDIR_INPUT,  m_ListPinsInput);
//hr = EnumPins(pFilter, PINDIR_OUTPUT, m_ListPinsOutput);
//pGB->ConnectDirect(ppinout,ppinin,pmt=NULL)
//枚举filter所有的pin
HRESULT EnumPins(IBaseFilter *pFilter, PIN_DIRECTION PinDir,
                              CListBox& Listbox)
{
    HRESULT hr;
    IEnumPins  *pEnum = NULL;
    IPin *pPin = NULL;

    // Clear the specified listbox (input or output)
    Listbox.ResetContent();

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        Listbox.AddString(TEXT("<ERROR>"));
        return hr;
    }

    // Enumerate all pins on this filter
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;

        hr = pPin->QueryDirection(&PinDirThis);
        if (FAILED(hr))
        {
            Listbox.AddString(TEXT("<ERROR>"));
            pPin->Release();
            continue;
        }

        // Does the pin's direction match the requested direction?
        if (PinDir == PinDirThis)
        {
            PIN_INFO pininfo={0};

            // Direction matches, so add pin name to listbox
            hr = pPin->QueryPinInfo(&pininfo);
            if (SUCCEEDED(hr))
            {
                CString str(pininfo.achName);
                Listbox.AddString(str);
            }

            // The pininfo structure contains a reference to an IBaseFilter,
            // so you must release its reference to prevent resource a leak.
            pininfo.pFilter->Release();
        }
        pPin->Release();
    }
    pEnum->Release();

    return hr;
}



//察看是否有属性页
BOOL SupportsPropertyPage(IBaseFilter *pFilter) 
{
    HRESULT hr;
//    TCHAR szNameToFind[128];
    ISpecifyPropertyPages *pSpecify;

    // Read the current filter name from the list box
    //int nCurSel = m_ListFilters.GetCurSel();
    //m_ListFilters.GetText(nCurSel, szNameToFind);
	
	if(!pFilter)return FALSE;
    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        pSpecify->Release();
        return TRUE;
    }
    else
        return FALSE;
}

//察看属性页
void OnButtonProppage(IBaseFilter *pFilter,HWND hWnd=ghApp) 
{
    HRESULT hr;
    //IBaseFilter *pFilter = NULL;
  //  TCHAR szNameToFind[128];
    ISpecifyPropertyPages *pSpecify;

	if(!pFilter)return ;

    // Read the current filter name from the list box
    //int nCurSel = m_ListFilters.GetCurSel();
    //m_ListFilters.GetText(nCurSel, szNameToFind);

    // Read the current list box name and find it in the graph
//    pFilter = FindFilterFromName(szNameToFind);
//    if (!pFilter)
//        return;

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        do 
        {
            FILTER_INFO FilterInfo;
            hr = pFilter->QueryFilterInfo(&FilterInfo);
            if (FAILED(hr))
                break;

            CAUUID caGUID;
            hr = pSpecify->GetPages(&caGUID);
            if (FAILED(hr))
                break;

            pSpecify->Release();
        
            // Display the filter's property page
            OleCreatePropertyFrame(
                hWnd,                 // Parent window
                0,                      // x (Reserved)
                0,                      // y (Reserved)
                FilterInfo.achName,     // Caption for the dialog box
                1,                      // Number of filters
                (IUnknown **)&pFilter,  // Pointer to the filter 
                caGUID.cElems,          // Number of property pages
                caGUID.pElems,          // Pointer to property page CLSIDs
                0,                      // Locale identifier
                0,                      // Reserved
                NULL                    // Reserved
            );
            CoTaskMemFree(caGUID.pElems);
            FilterInfo.pGraph->Release(); 

        } while(0);
    }

    pFilter->Release();
}

//把Graph保存为gif文件
HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath)
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph";
    HRESULT hr;
    IStorage *pStorage = NULL;

    hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr))
    {
        return hr;
    }

    IStream *pStream;
    hr = pStorage->CreateStream(
        wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStream);
    if (FAILED(hr))
    {
        pStorage->Release();
        return hr;
    }
	
    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream, (void**)&pPersist);
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr))
    {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}

//WINBASEAPI int WINAPI ::MultiByteToWideChar ( UINT CodePage=CP_ACP, DWORD dwFlags=0, LPCSTR lpMultiByteStr, int cchMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
void TToWide(LPTSTR szFile,WCHAR wFile[MAX_PATH])
{
    USES_CONVERSION;
    
    // Convert filename to wide character string
    wcsncpy(wFile, T2W(szFile), NUMELMS(wFile)-1);
    wFile[MAX_PATH-1] = 0;
}

void WideToT(LPTSTR szFile,WCHAR wFile[MAX_PATH])
{
    USES_CONVERSION;
    
    lstrcpy(szFile, W2A(wFile));
    szFile[MAX_PATH-1] = 0;
}



//下面的代码演示了如何利用CLSID生成一个filter，然后将其加入到graph图中
/*在你的应用程序中，你可以这样用这个函数

 IBaseFilter *pMux;
  hr = AddFilterByCLSID(pGraph, CLSID_AviDest, L"AVI Mux", &pMux); 
  
  if (SUCCEEDED(hr))
  {
  //* ... 
  pMux->Release();
  }
注：有些filter是不能通过with CoCreateInstance方法创建的。例如AVI Compressor Filter和WDM Video Capture filter*/
HRESULT AddFilterByCLSID(
  IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
  const GUID& clsid, // CLSID of the filter to create.
  LPCWSTR wszName, // A name for the filter.
  IBaseFilter **ppF) // Receives a pointer to the filter.
  {
  if (!pGraph || ! ppF) return E_POINTER;
  *ppF = 0;
  IBaseFilter *pF = 0;
  HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,
  IID_IBaseFilter, reinterpret_cast<void**>(&pF));
  if (SUCCEEDED(hr))
  {
  hr = pGraph->AddFilter(pF, wszName);
  if (SUCCEEDED(hr))
  *ppF = pF;
  else
  pF->Release();
  }
  return hr;
}
 

/*2如何查找filter空闲的pin
看代码把
下面的代码演示了如何利用上面的函数来在一个filter查找一个输出的空闲的pin。
 IPin *pOut = NULL;
  HRESULT hr = GetUnconnectedPin(pFilter, PINDIR_OUTPUT, &pOut);
  if (SUCCEEDED(hr))
  {
  //* ... 
  pOut->Release();
  }
  
*/
HRESULT GetUnconnectedPin(
						  IBaseFilter *pFilter, // Pointer to the filter.
						  PIN_DIRECTION PinDir, // Direction of the pin to find.
						  IPin **ppPin) // Receives a pointer to the pin.
{
	*ppPin = 0;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PinDir)
		{
			IPin *pTmp = 0;
			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr)) // Already connected, not the pin we want.
			{
				pTmp->Release();
			}
			else // Unconnected, 这就是我们想要的pin，空闲的pin
			{
				pEnum->Release();
				*ppPin = pPin;
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	// Did not find a matching pin.
	return E_FAIL;
}


/*3如何连接两个Filter
下面的函数演示了如何将一个filter的输出pin和另一个filter的第一个空闲的输入pin进行连接。*/
HRESULT ConnectFilters(
					   IGraphBuilder *pGraph, // Filter Graph Manager.
					   IPin *pOut, // Output pin on the upstream filter.
					   IBaseFilter *pDest) // Downstream filter.
{
	if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}
#ifdef debug
	PIN_DIRECTION PinDir;
	pOut->QueryDirection(&PinDir);
	_ASSERTE(PinDir == PINDIR_OUTPUT);
#endif
	//找一个空闲的输入pin
	IPin *pIn = 0;
	HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (FAILED(hr))
	{
		return hr;
	}
	// Try to connect them.
	hr = pGraph->Connect(pOut, pIn);
	pIn->Release();
	return hr;
}
/*下面是ConnectFilters的一个重载函数，但是第二个参数是一个指向filter的指针，而不是指向pin的指针，这个函数将两个filter连接起来。*/
HRESULT ConnectFilters(
					   IGraphBuilder *pGraph,
					   IBaseFilter *pSrc,
					   IBaseFilter *pDest)
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}
	// 首先在第一个filter上查询一个输出的pin接口
	IPin *pOut = 0;
	HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr))
	{
		return hr;
	}
	//然后将它和第二个filter的输入接口衔接。
	hr = ConnectFilters(pGraph, pOut, pDest);
	pOut->Release();
	return hr;
}

/*下面的函数演示了利用这个函数来连接AVIMux 过滤器和File Writer过滤器，这个例子也使用了AddFilterByCLSID函数。
 IBaseFilter *pMux, *pWrite;
  hr = AddFilterByCLSID(pGraph, CLSID_AviDest, L"AVI Mux", &pMux);
  if (SUCCEEDED(hr))
  {
  hr = AddFilterByCLSID(pGraph, CLSID_FileWriter, L"File Writer", &pWrite);
  if (SUCCEEDED(hr))
  {
  hr = ConnectFilters(pGraph, pMux, pWrite);
  //* Use IFileSinkFilter to set the file name (not shown). 
  pWrite->Release();
  }
  pMux->Release();
  }
  */


//4如何获得filter或者pin的接口指针
//一般来说，我们都是通过Filter图表管理器来进行一些操作，但是，有时候，我们也直接调用filter或者pin的一些方法，因此，我们需要获取filter或pin的接口指针。
//对于filter的接口指针，可以通过IEnumFilters来枚举filter的指针，看下面的代码把
HRESULT FindFilterInterface(
							IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
							REFGUID iid, // IID of the interface to retrieve.
							void **ppUnk) // Receives the interface pointer.
{
	if (!pGraph || !ppUnk) return E_POINTER;
	HRESULT hr = E_FAIL;
	IEnumFilters *pEnum = NULL;
	IBaseFilter *pF = NULL;
	if (FAILED(pGraph->EnumFilters(&pEnum)))
	{
		return E_FAIL;
	}
	// Query every filter for the interface.
	while (S_OK == pEnum->Next(1, &pF, 0))
	{
		hr = pF->QueryInterface(iid, ppUnk);
		pF->Release();
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	pEnum->Release();
	return hr;
}
//用IEnumPins来获得pin的接口指针，其实就是枚举哦
HRESULT FindPinInterface(
						 IBaseFilter *pFilter, // Pointer to the filter to search.
						 REFGUID iid, // IID of the interface.
						 void **ppUnk) // Receives the interface pointer.
{
	if (!pFilter || !ppUnk) return E_POINTER;
	HRESULT hr = E_FAIL;
	IEnumPins *pEnum = 0;
	if (FAILED(pFilter->EnumPins(&pEnum)))
	{
		return E_FAIL;
	}
	// Query every pin for the interface.
	IPin *pPin = 0;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		hr = pPin->QueryInterface(iid, ppUnk);
		pPin->Release();
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	pEnum->Release();
	return hr;
}
/*下面的代码演示了如何搜索任意的filter和pin的接口*/
HRESULT FindInterfaceAnywhere(
							  IGraphBuilder *pGraph,
							  REFGUID iid,
							  void **ppUnk)
{
	if (!pGraph || !ppUnk) return E_POINTER;
	HRESULT hr = E_FAIL;
	IEnumFilters *pEnum = 0;
	if (FAILED(pGraph->EnumFilters(&pEnum)))
	{
		return E_FAIL;
	}
	// Loop through every filter in the graph.
	IBaseFilter *pF = 0;
	while (S_OK == pEnum->Next(1, &pF, 0))
	{
		hr = pF->QueryInterface(iid, ppUnk);
		if (FAILED(hr))
		{
			// The filter does not expose the interface, but maybe
			// one of its pins does. //调用的是上面的搜索pin的函数
			hr = FindPinInterface(pF, iid, ppUnk);
		}
		pF->Release();
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	pEnum->Release();
	return hr;
}
//5如何查找和某个filter的上下相连的filter
//给你一个filter，你可以沿着graph图找到和它相联结的filter。首先枚举filter的pin，检查每一个pin是否有其他的pin的和它连接，如果有就检查连接pin属于哪个filter，你可以通过输入pin检查上游的filter，通过输出pin来检查下游的filter。
//下面的函数返回上游或者下游的和本filter连接的filter，只要有一个match，就返回。
 // Get the first upstream or downstream filter
HRESULT GetNextFilter(
					  IBaseFilter *pFilter, // 开始的filter
					  PIN_DIRECTION Dir, // 搜索的方向 (upstream 还是 downstream)
					  IBaseFilter **ppNext) // Receives a pointer to the next filter.
{
	if (!pFilter || !ppNext) return E_POINTER;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) return hr;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		// See if this pin matches the specified direction.
		PIN_DIRECTION ThisPinDir;
		hr = pPin->QueryDirection(&ThisPinDir);
		if (FAILED(hr))
		{
			// Something strange happened.
			hr = E_UNEXPECTED;
			pPin->Release();
			break;
		}
		if (ThisPinDir == Dir)
		{
			// Check if the pin is connected to another pin.
			IPin *pPinNext = 0;
			hr = pPin->ConnectedTo(&pPinNext);
			if (SUCCEEDED(hr))
			{
				// Get the filter that owns that pin.
				PIN_INFO PinInfo;
				hr = pPinNext->QueryPinInfo(&PinInfo);
				pPinNext->Release();
				pPin->Release();
				pEnum->Release();
				if (FAILED(hr) || (PinInfo.pFilter == NULL))
				{
					// Something strange happened.
					return E_UNEXPECTED;
				}
				// This is the filter we're looking for.
				*ppNext = PinInfo.pFilter; // Client must release.
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	// Did not find a matching filter.
	return E_FAIL;
}
//  下面演示如何使用这个函数
//  IBaseFilter *pF; // Pointer to some filter.
//  IBaseFilter *pUpstream = NULL;
//  if (SUCCEEDED(GetNextFilter(pF, PINDIR_INPUT, &pUpstream)))
//  {
//  // Use pUpstream ...
//  pUpstream->Release();
//  }
//但是，一个filter可能在某个方向同时连接着两个或者更多个filter，例如一个分割filter，就有好几个filter与之相联。
// 因此，你可能想将所有的filter通过一个集合都搜集到。下面的例子代码就演示了如何通过CGenericList结构来实现这个方法。
 #include <streams.h> // Link to the DirectShow base class library
  // Define a typedef for a list of filters.
typedef CGenericList<IBaseFilter> CFilterList;
// Forward declaration. Adds a filter to the list unless it's a duplicate.
void AddFilterUnique(CFilterList &FilterList, IBaseFilter *pNew);
// Find all the immediate upstream or downstream peers of a filter.
//输入pFilter，Dir，输出FilterList链表
HRESULT GetPeerFilters(
					   IBaseFilter *pFilter, // Pointer to the starting filter
					   PIN_DIRECTION Dir, // Direction to search (upstream or downstream)
					   CFilterList &FilterList) // Collect the results in this list.
{
	if (!pFilter) return E_POINTER;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) return hr;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		// See if this pin matches the specified direction.
		PIN_DIRECTION ThisPinDir;
		hr = pPin->QueryDirection(&ThisPinDir);
		if (FAILED(hr))
		{
			// Something strange happened.
			hr = E_UNEXPECTED;
			pPin->Release();
			break;
		}
		if (ThisPinDir == Dir)
		{
			// Check if the pin is connected to another pin.
			IPin *pPinNext = 0;
			hr = pPin->ConnectedTo(&pPinNext);
			if (SUCCEEDED(hr))
			{
				// Get the filter that owns that pin.
				PIN_INFO PinInfo;
				hr = pPinNext->QueryPinInfo(&PinInfo);
				pPinNext->Release();
				if (FAILED(hr) || (PinInfo.pFilter == NULL))
				{
					// Something strange happened.
					pPin->Release();
					pEnum->Release();
					return E_UNEXPECTED;
				}
				// 将符合的filter添加到list中
				AddFilterUnique(FilterList, PinInfo.pFilter);
				PinInfo.pFilter->Release();
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	return S_OK;
}
//子函数，把pNew添加到FilterList链表中
void AddFilterUnique(CFilterList &FilterList, IBaseFilter *pNew)
{
	if (pNew == NULL) return;
	POSITION pos = FilterList.GetHeadPosition();
	while (pos)
	{
		IBaseFilter *pF = FilterList.GetNext(pos);
		if (IsEqualObject(pF, pNew))
		{
			return;
		}
	}
	pNew->AddRef(); // The caller must release everything in the list.
	FilterList.AddTail(pNew);
}
//如何应用上面的函数呢？看看下面就知道了撒
//IBaseFilter *pF; // Pointer to some filter.
//  CFilterList FList(NAME("MyList")); // List to hold the downstream
//  peers.
//  hr = GetPeerFilters(pF, PINDIR_OUTPUT, FList);
//  if (SUCCEEDED(hr)) //解析filter 的集合。
//  {
//  POSITION pos = FList.GetHeadPosition();
//  while (pos)
//  {
//  IBaseFilter *pDownstream = FList.GetNext(pos);
//  pDownstream->Release();
//  }
//  }
/////////////////////////////////////////////////////////
//6如何删除graph中的所有filter
//很简单的，采用IFilterGraph::RemoveFilter函数
HRESULT RemoveAllFilters(IGraphBuilder *pGraph)
{ // Stop the graph.
	IMediaControl *pControl;
	HRESULT hr;
	JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pControl));
	pControl->Stop();
	// Enumerate the filters in the graph.
	IEnumFilters *pEnum = NULL;
	hr = pGraph->EnumFilters(&pEnum);
	if (SUCCEEDED(hr))
	{
		IBaseFilter *pFilter = NULL;
		while (S_OK == pEnum->Next(1, &pFilter, NULL))
		{
			// Remove the filter.
			pGraph->RemoveFilter(pFilter);
			// Reset the enumerator.
			pEnum->Reset();
			pFilter->Release();
		}
		pEnum->Release();
  }
	return hr;
}
//7如何利用Capture Graph Builder构建Graph图表
//Capture Graph Builder可以用来构建大多数的filter图表，并不仅仅是捕捉graph。本文简单介绍了如何利用Capture Graph Builder来构建graph。
//Capture Graph Builder暴露了ICaptureGraphBuilder2接口指针，首先创建一个capture builder，和一个filter图表管理器对象，然后用图表管理器对象指针初始化Capture Graph Builder。代码如下：
//IGraphBuilder *pGraph = NULL;
//  ICaptureGraphBuilder2 *pBuilder = NULL;
//// Create the Filter Graph Manager.
//  HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL,
//  CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
//if (SUCCEEDED(hr))
//  {
//  // Create the Capture Graph Builder.
//  hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
//  CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2,
//  (void **)&pBuilder);
//  if (SUCCEEDED(hr))
//  {
//  pBuilder->SetFiltergraph (pGraph);
//  }
//  };
//连接filter
//ICaptureGraphBuilder2::RenderStream方法可以同时将两个或者三个filter连接成一个链（chain）。通常情况下，当每个filter只有一个输出pin和一个输入pin时，这个方法就才，适用。
//我们现在先忽略前两个参数，第三个参数是一个IUnknown指针，指向一个filter或者输出pin。第五，六个参数指向IBaseFilter指针。RenderStream就将三个filter连接成一个链。例如，假设A，B，C 是三个filter，每个filter只有一个输出pin和一个输入pin。
//下面的代码可以将B连接到A上，将B连接到C上。
//RenderStream(NULL, NULL, A, B, C)
//所有的连接都是智能化的，如果是将两个filter相连，你可以将中间的参数设置为NULL，
//RenderStream(NULL, NULL, A, NULL, C)
//你也可以调用两次这个函数创建一个更长的链条。
//RenderStream(NULL, NULL, A, B, C)
//RenderStream(NULL, NULL, C, D, E)
//如果最后的一个参数设置为NULL，这个方法就自动的为graph设置一个renderer filter。如果是视频就设置成Video Renderer，如果是音频就设置为DirectSoundRenderer。因此
//RenderStream(NULL, NULL, A, NULL, NULL)
//等价于
//RenderStream(NULL, NULL, A, NULL, R)
//这里R指的是Render Filter。
//如果你在第三个参数指定的是filter，而不是pin，你就要在第一二个参数里指定使用那个输出pin用于连接。
//第一个参数只适用于捕捉filter，它指定pin的所属种类的GUID，具体的设置可以参考Pin Property Set.，但是下面的两个种类对于所有的filter都有效。
//PIN_CATEGORY_CAPTURE
//PIN_CATEGORY_PREVIEW
//如果捕捉filter不支持捕捉和预览，RenderStream方法就增加一个Smart Tee来分割数据流。
//如果播放文件，要将捕捉filter和一个mux filter连接起来，
//第二个参数指明了媒体类型
//MEDIATYPE_Audio
//MEDIATYPE_Video
//MEDIATYPE_Interleaved (DV)
//查询filter和pin的接口指针
//当你建立一个graph后，也许你需要查询graph中的filter和pin暴露的接口指针。例如，一个捕捉filter也许暴露了IAMDroppedFrames接口，它的输出pin也许暴露了IAMStreamConfig接口。
//查询接口最简单地方法就是使用ICaptureGraphBuilder2::FindInterface方法。这个方法遍历整个graph的filter和pin，直到他找到合适的filter。你可以指定开始的filter，然后指定搜索的方向，（向上搜索还是向下搜索）
//下面的代码在一个视频预览pin上搜索IAMStreamConfig接口
// IAMStreamConfig *pConfig = NULL;
//  HRESULT hr = pBuild->FindInterface(
//  &PIN_CATEGORY_PREVIEW,
//  &MEDIATYPE_Video,
//  pVCap,
//  IID_IAMStreamConfig,
//  (void**)&pConfig
//  );
//  if (SUCCESSFUL(hr))
//  {
//  /* ... */
//  pConfig->Release();
//  }
//查找pin
//如果你需要在某个filter上查询某个接口，可以用ICaptureGraphBuilder2::FindPin方法，代码如下：
// IPin *pPin = NULL;
//  hr = pBuild->FindPin(
//  pCap, // Pointer to the filter to search.
//  PINDIR_OUTPUT, // Search for an output pin.
//  &PIN_CATEGORY_PREVIEW, // Search for a preview pin.
//  &MEDIATYPE_Video, // Search for a video pin.
//  TRUE, // The pin must be unconnected.
//  0, // Return the first matching pin (index 0).
//  &pPin); // This variable receives the IPin pointer.
//  if (SUCCESSFUL(hr))
//  {
//  /* ... */
//  pPin->Release();
//  }



//显示属性页
void ShowProppage(IBaseFilter *pFilter,HWND hWnd) 
{
    HRESULT hr;
    //IBaseFilter *pFilter = NULL;
//    TCHAR szNameToFind[128];
    ISpecifyPropertyPages *pSpecify;

    // Read the current filter name from the list box
//    int nCurSel = m_ListFilters.GetCurSel();
//    m_ListFilters.GetText(nCurSel, szNameToFind);
//
//    // Read the current list box name and find it in the graph
//    pFilter = FindFilterFromName(szNameToFind);
    if (!pFilter)
        return;

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        do 
        {
            FILTER_INFO FilterInfo;
            hr = pFilter->QueryFilterInfo(&FilterInfo);
            if (FAILED(hr))
                break;

            CAUUID caGUID;
            hr = pSpecify->GetPages(&caGUID);
            if (FAILED(hr))
                break;

            pSpecify->Release();
        
            // Display the filter's property page
            OleCreatePropertyFrame(
                hWnd,                 // Parent window
                0,                      // x (Reserved)
                0,                      // y (Reserved)
                FilterInfo.achName,     // Caption for the dialog box
                1,                      // Number of filters
                (IUnknown **)&pFilter,  // Pointer to the filter 
                caGUID.cElems,          // Number of property pages
                caGUID.pElems,          // Pointer to property page CLSIDs
                0,                      // Locale identifier
                0,                      // Reserved
                NULL                    // Reserved
            );
            CoTaskMemFree(caGUID.pElems);
            LIF(FilterInfo.pGraph->Release()); //这一句不知道是干什么的，有时候会出错

        } while(0);
    }

    pFilter->Release();
}