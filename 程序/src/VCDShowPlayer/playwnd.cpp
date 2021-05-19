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

//���ϵ�ȫ�ֱ���һ��������
//�ڴ����д��ļ�����
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
//��ʼ�����ڴ�С
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

//pVW��ghApp
//ʹ��Ƶ���ںͳ��򴰿�һ����
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

//�ж��Ƿ�΢��Ķ�ý���ļ�
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

//���ļ������ţ�����
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
//����һ�����ļ����ڣ������ļ�����
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
//�ͷ�����DSHow����pVW  g_dwGraphRegister pME ������
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
//��ӵ�Rot�У�pdwRegister�ƺ���һ����ʾע��ID�ı���
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
//�Ƴ�GraphFromRot
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

//����һ����Ϣ��
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

//����ȫ·��pszFull�������ļ���pszFile
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
//��ȫ���ͷ�ȫ���л�
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
//����Ƿ�֧�ֲ���������Ǿͳ�ʼ��pFS
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

//pFS ��FILTER_STATE g_psCurrent
//���� 
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
//�����ಽ
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

//pMP UpdateMainTitle����
//�޸Ĳ����ٶ�
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
//����Graph�ĸ����¼�WM_GRAPHNOTIFY
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

	//PlayMovieInWindow("f:\\����.DAT.mpg");
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
			//if�������Ĳ���ʱȡ��buffer��size��
			//�������ڲ�ȷ��image buffer�Ĵ�С������£����Ǹ�
			//GetCurrentImage�ĵڶ�����������0����NULL��ȡ��buffer��
			//��С���Ժ�ʹ�á�
			bool pass = false; 
			unsigned char * buffer = new unsigned char[bitmapSize]; 
			if(SUCCEEDED(mBasicVideo->GetCurrentImage(&bitmapSize,(long*)buffer))) 
			{ 
				//��ʱ�Ѿ��õ��ղ���ȡ�õĴ�С������ռ䣩
				BITMAPFILEHEADER  hdr;    //Bitmap��ͷ��Ϣ
				LPBITMAPINFOHEADER   lpbi; // Bitmap���ļ���Ϣ���������ݣ�
				
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
			delete [] buffer; //�����ù�֮��ǵ�Ҫ�ͷſռ�
			return true; 
		} 
	} 
	
	return false; 
} 

//pGB
//����filter�����֣�����IBaseFilterָ��
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
//ö��filter���е�pin
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



//�쿴�Ƿ�������ҳ
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

//�쿴����ҳ
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

//��Graph����Ϊgif�ļ�
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



//����Ĵ�����ʾ���������CLSID����һ��filter��Ȼ������뵽graphͼ��
/*�����Ӧ�ó����У�������������������

 IBaseFilter *pMux;
  hr = AddFilterByCLSID(pGraph, CLSID_AviDest, L"AVI Mux", &pMux); 
  
  if (SUCCEEDED(hr))
  {
  //* ... 
  pMux->Release();
  }
ע����Щfilter�ǲ���ͨ��with CoCreateInstance���������ġ�����AVI Compressor Filter��WDM Video Capture filter*/
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
 

/*2��β���filter���е�pin
�������
����Ĵ�����ʾ�������������ĺ�������һ��filter����һ������Ŀ��е�pin��
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
			else // Unconnected, �����������Ҫ��pin�����е�pin
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


/*3�����������Filter
����ĺ�����ʾ����ν�һ��filter�����pin����һ��filter�ĵ�һ�����е�����pin�������ӡ�*/
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
	//��һ�����е�����pin
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
/*������ConnectFilters��һ�����غ��������ǵڶ���������һ��ָ��filter��ָ�룬������ָ��pin��ָ�룬�������������filter����������*/
HRESULT ConnectFilters(
					   IGraphBuilder *pGraph,
					   IBaseFilter *pSrc,
					   IBaseFilter *pDest)
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}
	// �����ڵ�һ��filter�ϲ�ѯһ�������pin�ӿ�
	IPin *pOut = 0;
	HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr))
	{
		return hr;
	}
	//Ȼ�����͵ڶ���filter������ӿ��νӡ�
	hr = ConnectFilters(pGraph, pOut, pDest);
	pOut->Release();
	return hr;
}

/*����ĺ�����ʾ�������������������AVIMux ��������File Writer���������������Ҳʹ����AddFilterByCLSID������
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


//4��λ��filter����pin�Ľӿ�ָ��
//һ����˵�����Ƕ���ͨ��Filterͼ�������������һЩ���������ǣ���ʱ������Ҳֱ�ӵ���filter����pin��һЩ��������ˣ�������Ҫ��ȡfilter��pin�Ľӿ�ָ�롣
//����filter�Ľӿ�ָ�룬����ͨ��IEnumFilters��ö��filter��ָ�룬������Ĵ����
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
//��IEnumPins�����pin�Ľӿ�ָ�룬��ʵ����ö��Ŷ
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
/*����Ĵ�����ʾ��������������filter��pin�Ľӿ�*/
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
			// one of its pins does. //���õ������������pin�ĺ���
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
//5��β��Һ�ĳ��filter������������filter
//����һ��filter�����������graphͼ�ҵ������������filter������ö��filter��pin�����ÿһ��pin�Ƿ���������pin�ĺ������ӣ�����оͼ������pin�����ĸ�filter�������ͨ������pin������ε�filter��ͨ�����pin��������ε�filter��
//����ĺ����������λ������εĺͱ�filter���ӵ�filter��ֻҪ��һ��match���ͷ��ء�
 // Get the first upstream or downstream filter
HRESULT GetNextFilter(
					  IBaseFilter *pFilter, // ��ʼ��filter
					  PIN_DIRECTION Dir, // �����ķ��� (upstream ���� downstream)
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
//  ������ʾ���ʹ���������
//  IBaseFilter *pF; // Pointer to some filter.
//  IBaseFilter *pUpstream = NULL;
//  if (SUCCEEDED(GetNextFilter(pF, PINDIR_INPUT, &pUpstream)))
//  {
//  // Use pUpstream ...
//  pUpstream->Release();
//  }
//���ǣ�һ��filter������ĳ������ͬʱ�������������߸����filter������һ���ָ�filter�����кü���filter��֮������
// ��ˣ�������뽫���е�filterͨ��һ�����϶��Ѽ�������������Ӵ������ʾ�����ͨ��CGenericList�ṹ��ʵ�����������
 #include <streams.h> // Link to the DirectShow base class library
  // Define a typedef for a list of filters.
typedef CGenericList<IBaseFilter> CFilterList;
// Forward declaration. Adds a filter to the list unless it's a duplicate.
void AddFilterUnique(CFilterList &FilterList, IBaseFilter *pNew);
// Find all the immediate upstream or downstream peers of a filter.
//����pFilter��Dir�����FilterList����
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
				// �����ϵ�filter��ӵ�list��
				AddFilterUnique(FilterList, PinInfo.pFilter);
				PinInfo.pFilter->Release();
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	return S_OK;
}
//�Ӻ�������pNew��ӵ�FilterList������
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
//���Ӧ������ĺ����أ����������֪������
//IBaseFilter *pF; // Pointer to some filter.
//  CFilterList FList(NAME("MyList")); // List to hold the downstream
//  peers.
//  hr = GetPeerFilters(pF, PINDIR_OUTPUT, FList);
//  if (SUCCEEDED(hr)) //����filter �ļ��ϡ�
//  {
//  POSITION pos = FList.GetHeadPosition();
//  while (pos)
//  {
//  IBaseFilter *pDownstream = FList.GetNext(pos);
//  pDownstream->Release();
//  }
//  }
/////////////////////////////////////////////////////////
//6���ɾ��graph�е�����filter
//�ܼ򵥵ģ�����IFilterGraph::RemoveFilter����
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
//7�������Capture Graph Builder����Graphͼ��
//Capture Graph Builder�������������������filterͼ�����������ǲ�׽graph�����ļ򵥽������������Capture Graph Builder������graph��
//Capture Graph Builder��¶��ICaptureGraphBuilder2�ӿ�ָ�룬���ȴ���һ��capture builder����һ��filterͼ�����������Ȼ����ͼ�����������ָ���ʼ��Capture Graph Builder���������£�
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
//����filter
//ICaptureGraphBuilder2::RenderStream��������ͬʱ��������������filter���ӳ�һ������chain����ͨ������£���ÿ��filterֻ��һ�����pin��һ������pinʱ����������Ͳţ����á�
//���������Ⱥ���ǰ����������������������һ��IUnknownָ�룬ָ��һ��filter�������pin�����壬��������ָ��IBaseFilterָ�롣RenderStream�ͽ�����filter���ӳ�һ���������磬����A��B��C ������filter��ÿ��filterֻ��һ�����pin��һ������pin��
//����Ĵ�����Խ�B���ӵ�A�ϣ���B���ӵ�C�ϡ�
//RenderStream(NULL, NULL, A, B, C)
//���е����Ӷ������ܻ��ģ�����ǽ�����filter����������Խ��м�Ĳ�������ΪNULL��
//RenderStream(NULL, NULL, A, NULL, C)
//��Ҳ���Ե������������������һ��������������
//RenderStream(NULL, NULL, A, B, C)
//RenderStream(NULL, NULL, C, D, E)
//�������һ����������ΪNULL������������Զ���Ϊgraph����һ��renderer filter���������Ƶ�����ó�Video Renderer���������Ƶ������ΪDirectSoundRenderer�����
//RenderStream(NULL, NULL, A, NULL, NULL)
//�ȼ���
//RenderStream(NULL, NULL, A, NULL, R)
//����Rָ����Render Filter��
//������ڵ���������ָ������filter��������pin�����Ҫ�ڵ�һ����������ָ��ʹ���Ǹ����pin�������ӡ�
//��һ������ֻ�����ڲ�׽filter����ָ��pin�����������GUID����������ÿ��Բο�Pin Property Set.�������������������������е�filter����Ч��
//PIN_CATEGORY_CAPTURE
//PIN_CATEGORY_PREVIEW
//�����׽filter��֧�ֲ�׽��Ԥ����RenderStream����������һ��Smart Tee���ָ���������
//��������ļ���Ҫ����׽filter��һ��mux filter����������
//�ڶ�������ָ����ý������
//MEDIATYPE_Audio
//MEDIATYPE_Video
//MEDIATYPE_Interleaved (DV)
//��ѯfilter��pin�Ľӿ�ָ��
//���㽨��һ��graph��Ҳ������Ҫ��ѯgraph�е�filter��pin��¶�Ľӿ�ָ�롣���磬һ����׽filterҲ��¶��IAMDroppedFrames�ӿڣ��������pinҲ��¶��IAMStreamConfig�ӿڡ�
//��ѯ�ӿ���򵥵ط�������ʹ��ICaptureGraphBuilder2::FindInterface���������������������graph��filter��pin��ֱ�����ҵ����ʵ�filter�������ָ����ʼ��filter��Ȼ��ָ�������ķ��򣬣�����������������������
//����Ĵ�����һ����ƵԤ��pin������IAMStreamConfig�ӿ�
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
//����pin
//�������Ҫ��ĳ��filter�ϲ�ѯĳ���ӿڣ�������ICaptureGraphBuilder2::FindPin�������������£�
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



//��ʾ����ҳ
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
            LIF(FilterInfo.pGraph->Release()); //��һ�䲻֪���Ǹ�ʲô�ģ���ʱ������

        } while(0);
    }

    pFilter->Release();
}