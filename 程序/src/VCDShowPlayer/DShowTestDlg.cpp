// DShowTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include <dshow.h>
#include <atlbase.h>


#include "DShowTest.h"

#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "InfoDlg.h"
#include "playwnd.h"
CString szInfo;
#include <initguid.h> 
DEFINE_GUID(CLSID_BouncingBall,  // Pasted from ball.h
		0xfd501041, 0x8ebe, 0x11ce, 0x81, 0x83, 0x00, 0xaa, 0x00, 0x57, 0x7d, 
		0xa1);


// { 8B498501-1218-11cf-ADC4-00A0D100041B }
DEFINE_GUID(CLSID_EZrgb24,
0x8b498501, 0x1218, 0x11cf, 0xad, 0xc4, 0x0, 0xa0, 0xd1, 0x0, 0x4, 0x1b);
// And the property page we support

// { 8B498502-1218-11cf-ADC4-00A0D100041B }
DEFINE_GUID(CLSID_EZrgb24PropertyPage,
0x8b498502, 0x1218, 0x11cf, 0xad, 0xc4, 0x0, 0xa0, 0xd1, 0x0, 0x4, 0x1b);
//WINBASEAPI int WINAPI MultiByteToWideChar ( UINT CodePage, DWORD dwFlags, 
//	LPCSTR lpMultiByteStr, int cchMultiByte, LPWSTR lpWideCharStr, int cchWideChar)

#include <strmif.h>
#include "..\iEZ.h"

#include "DShowTestDlg.h"
extern ICaptureGraphBuilder2 * pCapture ;
extern PLAYSTATE psCurrent ;

WCHAR g_wGrfFile[MAX_PATH];
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDShowTestDlg dialog

CDShowTestDlg::CDShowTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDShowTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDShowTestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bCanChgEffect=FALSE;
	m_pIPEffect=0;
	//WinExec("regsvr32 ezrgb24.ax",0);
}

void CDShowTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDShowTestDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDShowTestDlg, CDialog)
	//{{AFX_MSG_MAP(CDShowTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDM_PLAYFILE, OnPlayfile)
	ON_BN_CLICKED(IDM_Test, OnTest)
	ON_BN_CLICKED(IDC_PLAYVIDEO, OnPlayvideo)
	ON_BN_CLICKED(IDC_LISTFILTERS, OnListfilters)
	ON_BN_CLICKED(IDC_SNAPPIC, OnSnappic)
	ON_BN_CLICKED(IDC_LISTFILTERBYGRAPH, OnListfilterbygraph)
	ON_BN_CLICKED(IDC_TESTBALL, OnTestball)
	ON_BN_CLICKED(IDC_TestOurFilter, OnTestOurFilter)
	ON_BN_CLICKED(IDC_SAVE_GIF, OnSaveGif)
	ON_WM_SIZE()
	ON_COMMAND(IDM_NONE, OnEffectNone)
	ON_UPDATE_COMMAND_UI(IDM_NONE, OnUpdateNone)
	ON_COMMAND(IDC_TRACKING_PREV_ASBKGND, OnTrackingPrevAsbkgnd)
	ON_COMMAND(IDC_LIGHT_IMPROVED, OnLightImproved)
	ON_COMMAND(IDC_LIGHT_RECTANGLE, OnLightRectangle)
	ON_COMMAND(IDC_TRACKING_DYNAMIC_BKGND, OnTrackingDynamicBkgnd)
	ON_COMMAND(IDC_TRACKING_FIRST_ASBKGND, OnTrackingFirstAsbkgnd)
	ON_COMMAND(IDC_GREY, OnGrey)
	ON_COMMAND(IDC_BLUE, OnBlue)
	ON_COMMAND(IDC_BLUR, OnBlur)
	ON_COMMAND(IDC_DARKEN, OnDarken)
	ON_COMMAND(IDC_EMBOSS, OnEmboss)
	ON_COMMAND(IDC_GREEN, OnGreen)
	ON_COMMAND(IDC_POSTERIZE, OnPosterize)
	ON_COMMAND(IDC_RED, OnRed)
	ON_COMMAND(IDC_XOR, OnXor)
	ON_COMMAND(IDC_LIGHT_RECTANGLE2, OnLightRectangle2)
	ON_COMMAND(IDM_STOP, OnStop)
	ON_COMMAND(IDM_RUN, OnRun)
	ON_COMMAND(IDM_PAUSE, OnPause)
	ON_COMMAND(IDM_CapturePreview, OnCapturePreview)
	ON_COMMAND(IDC_LISTFILTERBYGRAPH, OnListfilterbygraph)
	ON_COMMAND(IDC_LISTFILTERS, OnListfilters)
	ON_COMMAND(IDC_PLAYVIDEO, OnPlayvideo)
	ON_COMMAND(IDM_TEST_INTERFACE, OnTestInterface)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDShowTestDlg message handlers

BOOL CDShowTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
//	添加工具条
//		if (!m_wndtoolbar.CreateEx( this,TBSTYLE_FLAT,  WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS ,
//			CRect(4,4,0,0)) ||	!m_wndtoolbar.LoadToolBar(IDR_TOOLBAR1) )
//		{
//			TRACE0("failed to create toolbar\n");
//			return FALSE;
//		}
//		m_wndtoolbar.ShowWindow(SW_SHOW);
//		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	CoInitialize(0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDShowTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDShowTestDlg::OnPaint() 
{
	CPaintDC dc(this);
	if (IsIconic())
	{
		 // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
//		CRect rect; 
//		//GetClientRect(rect); 
//		::GetClientRect(GetDlgItem(IDM_PIC)->m_hWnd,&rect);
//		dc.FillSolidRect(rect, RGB(0xff,0xff,0xff)); 
//		CDialog::OnPaint();
		
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDShowTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
//不要了...
void CDShowTestDlg::OnPlayfile() 
{
	// TODO: Add your control notification handler code here
	IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaEvent   *pEvent = NULL;

    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        AfxMessageBox("ERROR - Could not initialize COM library");
        return;
    }

    // Create the filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr))
    {
        AfxMessageBox("ERROR - Could not create the Filter Graph Manager.");
        return;
    }

    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

    // Build the graph. IMPORTANT: Change this string to a file on your system.
//	CFileDialog dlg(TRUE);
//	if(dlg.DoModal()!=IDOK)return;
//	WCHAR wcPath[260];
//	WideCharToMultiByte(CP_ACP, 0, wcPath, -1, (LPSTR)(LPCSTR)dlg.GetPathName(), MAX_PATH, NULL, NULL);

	IVideoWindow *pVW1;
	(pGraph->QueryInterface(IID_IVideoWindow,(void**)&pVW1));

	HWND hWndVideo=::GetDlgItem(m_hWnd,IDM_PIC);
//	TRACE("%d\n",hWndVideo);
	pVW1->put_Owner((OAHWND)(hWndVideo));
	pVW1->put_WindowStyle(WS_CHILD);
//	pVW->SetWindowPosition(0,0,100,100);
//	pVW->put_Visible(OATRUE);
//不好使，奇怪

//	IVMRWindowlessControl *pWc = NULL;
//	hr = InitWindowlessVMR(m_hWnd, pGraph, &pWc);
//	if (!SUCCEEDED(hr))
//	{
//		AfxMessageBox("Error");
//	}
//也不好使！显示不出它来
	
    hr = pGraph->RenderFile(L"f:\\花香.DAT.mpg", NULL);
    if (SUCCEEDED(hr))
    {
        // Run the graph.
		AfxMessageBox("start run");
        hr = pControl->Run();
        if (SUCCEEDED(hr))
        {
            // Wait for completion.
            long evCode;
            pEvent->WaitForCompletion(INFINITE, &evCode);

            // Note: Do not use INFINITE in a real application, because it
            // can block indefinitely.
        }
    }
    pControl->Release();
    pEvent->Release();
    pGraph->Release();
    CoUninitialize();
}

void CDShowTestDlg::OnTest() 
{
	// TODO: Add your control notification handler code here
	
	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        AfxMessageBox("ERROR - Could not initialize COM library");
        return;
    }

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		//return ;
	}
	
	CString s;
	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &pEnumCat, 0);
	
	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					s+=varName.bstrVal;
				}
				VariantClear(&varName);
				
				// To create an instance of the filter, do the following:
				IBaseFilter *pFilter;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pFilter);
				// Now add the filter to the graph. 
				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	
	//AfxMessageBox(s + "xx");
	CInfoDlg dlg;
	dlg.m_Info=s;
	dlg.DoModal();
}


void CDShowTestDlg::OnPlayvideo() 
{
	// TODO: Add your control notification handler code here

	CloseInterfaces();
	CloseClip();

	//初始化，很重要啊！
	//InitVideo(::GetDlgItem(AfxGetMainWnd()->m_hWnd,IDM_PIC));
	CoInitialize(0);
	ghApp=m_hWnd;

	
//	hr = AddGraphToRot(pGB, &g_dwGraphRegister);
//	if (FAILED(hr))
//	{
//		Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
//		g_dwGraphRegister = 0;
//	}//在PlayMovieInWindow中已经声明了

	TCHAR szFilename[MAX_PATH];
	if(!GetClipFileName(szFilename))
	{
		DWORD dwDlgErr = CommDlgExtendedError();
		
		// Don't show output if user cancelled the selection (no dlg error)
		if (dwDlgErr)
		{
			Msg(TEXT("GetClipFileName Failed! Error=0x%x\r\n"), GetLastError());
		}
		return;
	}
	
	
	lstrcpy(g_szFileName, szFilename);
	
	//播放，同样很重要！
	PlayMovieInWindow(g_szFileName);
//	g_psCurrent=Running;
//	g_bFullscreen = FALSE;
//    g_PlaybackRate = 1.0;
	//SetFocus(ghApp);
//	InitVideoWindow(1,1);
//	ModifyRate(0.25);
//	SetRate(1.0);
/*case WM_GRAPHNOTIFY:
            HandleGraphEvent();
            break;*/
	
}




void ShowFilenameByCLSID(REFCLSID clsid)
{
    HRESULT hr;
    LPOLESTR strCLSID;

    // Convert binary CLSID to a readable version
    hr = StringFromCLSID(clsid, &strCLSID);
    if(SUCCEEDED(hr))
    {
        TCHAR szKey[512];
        CString strQuery(strCLSID);

        // Create key name for reading filename registry
        wsprintf(szKey, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"),
                 (LPCTSTR) strQuery);

        // Free memory associated with strCLSID (allocated in StringFromCLSID)
        CoTaskMemFree(strCLSID);

        HKEY hkeyFilter=0;
        DWORD dwSize=MAX_PATH;
        BYTE pbFilename[MAX_PATH];
        int rc=0;

        // Open the CLSID key that contains information about the filter
        rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
        if (rc == ERROR_SUCCESS)
        {
            rc = RegQueryValueEx(hkeyFilter, NULL,  // Read (Default) value
                                 NULL, NULL, pbFilename, &dwSize);

            if (rc == ERROR_SUCCESS)
            {
                TCHAR szFilename[MAX_PATH];
                wsprintf(szFilename, TEXT("%s\0"), pbFilename);
//                m_StrFilename.SetWindowText(szFilename);
					szInfo+="        ";
					szInfo+=szFilename;
					szInfo+="\r\n";
            }

            RegCloseKey(hkeyFilter);
        }
    }
}

int DispFilters(CLSID* clsid)
{	
	HRESULT hr;    
    IEnumMoniker *pEnumCat = NULL;
	ICreateDevEnum* m_pSysDevEnum=NULL;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
                          CLSCTX_INPROC, IID_ICreateDevEnum, 
                          (void **)&m_pSysDevEnum);
    if FAILED(hr)
    {
        CoUninitialize();
        return FALSE;
    }
	
	// Enumerate all filters of the selected category  
    hr = m_pSysDevEnum->CreateClassEnumerator(*clsid, &pEnumCat, 0);
    ASSERT(SUCCEEDED(hr));
    if FAILED(hr)
        return -1;
	
    // Enumerate all filters using the category enumerator
    //hr = EnumFilters(pEnumCat);
	{
		HRESULT hr=S_OK;
		IMoniker *pMoniker;
		ULONG cFetched;
		VARIANT varName={0};
		int nFilters=0;
		
		// Clear the current filter list
//		ClearFilterList();
		
		// If there are no filters of a requested type, show default string
		if (!pEnumCat)
		{
//			m_FilterList.AddString(TEXT("<< No entries >>"));
//			SetNumFilters(nFilters);
			return S_FALSE;
		}
		
		// Enumerate all items associated with the moniker
		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			ASSERT(pMoniker);
			
			// Associate moniker with a file
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			ASSERT(SUCCEEDED(hr));
			ASSERT(pPropBag);
			if (FAILED(hr))
				continue;
			
			// Read filter name from property bag
			varName.vt = VT_BSTR;
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			if (FAILED(hr))
				continue;
			
			// Get filter name (converting BSTR name to a CString)
			CString str(varName.bstrVal);
			SysFreeString(varName.bstrVal);
			nFilters++;
			
			// Read filter's CLSID from property bag.  This CLSID string will be
			// converted to a binary CLSID and passed to AddFilter(), which will
			// add the filter's name to the listbox and its CLSID to the listbox
			// item's DataPtr item.  When the user clicks on a filter name in
			// the listbox, we'll read the stored CLSID, convert it to a string,
			// and use it to find the filter's filename in the registry.
			VARIANT varFilterClsid;
			varFilterClsid.vt = VT_BSTR;
			
			// Read CLSID string from property bag
			hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);
			if(SUCCEEDED(hr))
			{
				CLSID clsidFilter;
				
				// Add filter name and CLSID to listbox
				if(CLSIDFromString(varFilterClsid.bstrVal, &clsidFilter) == S_OK)
				{
					//AddFilter(str, &clsidFilter);
					szInfo+="    ";
					szInfo+=str;
					szInfo+="\r\n";
					ShowFilenameByCLSID(clsidFilter);
				}
				
				SysFreeString(varFilterClsid.bstrVal);
			}
			
			// Cleanup interfaces
			SAFE_RELEASE(pPropBag);
			SAFE_RELEASE(pMoniker);
		}
		
		// Update count of enumerated filters
		//SetNumFilters(nFilters);
	}
}
void CDShowTestDlg::OnListfilters() 
{
	// TODO: Add your control notification handler code here
	//CString szInfo;
	szInfo="";
	CoInitialize(NULL);
	USES_CONVERSION;

    HRESULT hr;
    IEnumMoniker *pEmCat = 0;
    ICreateDevEnum *pCreateDevEnum = NULL;
    int nClasses=0;

    // Create an enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return;

    // Use the meta-category that contains a list of all categories.
    // This emulates the behavior of GraphEdit.
    hr = pCreateDevEnum->CreateClassEnumerator(
                         CLSID_ActiveMovieCategories, &pEmCat, 0);
    ASSERT(SUCCEEDED(hr));

    if(hr == S_OK)
    {
        IMoniker *pMCat;
        ULONG cFetched;

        // Enumerate over every category
        while(hr = pEmCat->Next(1, &pMCat, &cFetched),
              hr == S_OK)
        {
            IPropertyBag *pPropBag;

            // Associate moniker with a file
            hr = pMCat->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if(SUCCEEDED(hr))
            {
                VARIANT varCatClsid;
                varCatClsid.vt = VT_BSTR;

                // Read CLSID string from property bag
                hr = pPropBag->Read(L"CLSID", &varCatClsid, 0);
                if(SUCCEEDED(hr))
                {
                    CLSID clsidCat;

                    if(CLSIDFromString(varCatClsid.bstrVal, &clsidCat) == S_OK)
                    {
                        // Use the guid if we can't get the name
                        WCHAR *wszCatName;
                        TCHAR szCatDesc[MAX_PATH];

                        VARIANT varCatName;
                        varCatName.vt = VT_BSTR;

                        // Read filter name
                        hr = pPropBag->Read(L"FriendlyName", &varCatName, 0);
                        if(SUCCEEDED(hr))
                            wszCatName = varCatName.bstrVal;
                        else
                            wszCatName = varCatClsid.bstrVal;

#ifndef UNICODE
                        WideCharToMultiByte(
                                CP_ACP, 0, wszCatName, -1,
                                szCatDesc, sizeof(szCatDesc), 0, 0);
#else
                        lstrcpy(szCatDesc, W2T(wszCatName));
#endif

                        if(SUCCEEDED(hr))
                            SysFreeString(varCatName.bstrVal);

                        // Add category name and CLSID to list box
                        //AddFilterCategory(szCatDesc, &clsidCat);
						szInfo +=szCatDesc;
						szInfo+="\r\n";
						DispFilters(&clsidCat);
						
                        nClasses++;
                    }

                    SysFreeString(varCatClsid.bstrVal);
                }

                pPropBag->Release();
            }
            else
            {
                break;
            }

            pMCat->Release();
        } // for loop

        pEmCat->Release();
    }

    pCreateDevEnum->Release();

    // Update listbox title with number of classes
//    SetNumClasses(nClasses);
	CInfoDlg dlg;
	dlg.m_Info=szInfo;
	dlg.DoModal();
}


extern bool SnapImage(IBasicVideo *mBasicVideo, TCHAR *szFilename) ;
void CDShowTestDlg::OnSnappic() 
{
	// TODO: Add your control notification handler code here
	if(pBV)
	{
		SnapImage(pBV,"c:\\test.bmp");
		AfxMessageBox("ok");
	}
}

void CDShowTestDlg::OnListfilterbygraph() 
{

    USES_CONVERSION;

    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;

	if(!pGB)return;
	szInfo="";
    // Clear filters list box
   // m_ListFilters.ResetContent();
    
    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
    {
        //m_ListFilters.AddString(TEXT("<ERROR>"));
        return ;
    }

    // Enumerate all filters in the graph
    while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
    {
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
        
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (FAILED(hr))
        {
            //m_ListFilters.AddString(TEXT("<ERROR>"));
        }
        else
        {
            // Add the filter name to the filters listbox
            lstrcpy(szName, W2A(FilterInfo.achName));
            //m_ListFilters.AddString(szName);

			szInfo+=szName;
			szInfo+="\r\n";
            FilterInfo.pGraph->Release();
        }       
        pFilter->Release();
    }
    pEnum->Release();
	
	CInfoDlg dlg;
	dlg.m_Info=szInfo;
	dlg.DoModal();
    return ;

	// TODO: Add your control notification handler code here
	
}

//以下是加载filter,render filter的一个例子

	
void CDShowTestDlg::OnTestball() 
{
	USES_CONVERSION;
    HRESULT hr;
	
	CloseInterfaces();
	CloseClip();

	
	InitVideo(::GetDlgItem(AfxGetMainWnd()->m_hWnd,IDM_PIC));

	CoInitialize(0);
    
    // Get the interface for DirectShow's GraphBuilder
    LIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&pGB));

    if(SUCCEEDED(hr))
    {
        // Have the graph builder construct its the appropriate graph automatically
       // LIF(pGB->RenderFile(wFile, NULL));

        // QueryInterface for DirectShow interfaces
        LIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        LIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
        LIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));
        LIF(pGB->QueryInterface(IID_IMediaPosition, (void **)&pMP));

        // Query for video interfaces, which may not be relevant for audio files
        LIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));
        LIF(pGB->QueryInterface(IID_IBasicVideo, (void **)&pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        LIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

        
        // Have the graph signal event via window callbacks for performance
		//#define WM_GRAPHNOTIFY WM_USER+13
        LIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));


#ifdef REGISTER_FILTERGRAPH
        hr = AddGraphToRot(pGB, &g_dwGraphRegister);
        if (FAILED(hr))
        {
            Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
            g_dwGraphRegister = 0;
        }
#endif

    }
	//////////////////////////////////
	// Create a new instance of the Ball filter and add it to the graph.
	IBaseFilter *pBall = 0;
	hr = CoCreateInstance(CLSID_BouncingBall, 0, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, reinterpret_cast<void**>(&pBall));
	hr = pGB->AddFilter(pBall, L"Bouncing Ball");
	pBall->Release();
	
	// Now render every output pin.
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	pBall->EnumPins(&pEnum);
	while (hr = pEnum->Next(1, &pPin, NULL), hr == S_OK)
	{
		// Check the pin direction.
		PIN_DIRECTION PinDir;
		pPin->QueryDirection(&PinDir);
		if (PinDir == PINDIR_OUTPUT)
		{
			AfxMessageBox("StartRender");
//			LIF(pVW->put_Owner((OAHWND)ghApp));
//            LIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));
			hr = pGB->Render(pPin);
			pMC->Run();
		}
		pPin->Release();
	}
	pEnum->Release();
	pBall->Release();
}

//测试自己做的那个filter，只是不知道怎么用代码切换filter的特效
// Special effects filter CLSID

void CDShowTestDlg::OnTestOurFilter() 
{
	
	CloseInterfaces();
	CloseClip();
	
	//初始化，很重要啊！
	//InitVideo(::GetDlgItem(AfxGetMainWnd()->m_hWnd,IDM_PIC));
	InitVideo(AfxGetMainWnd()->m_hWnd);
	CoInitialize(0);
	
	TCHAR szFile[MAX_PATH];
	if(!GetClipFileName(szFile))
	{
		DWORD dwDlgErr = CommDlgExtendedError();
		
		// Don't show output if user cancelled the selection (no dlg error)
		if (dwDlgErr)
		{
			Msg(TEXT("GetClipFileName Failed! Error=0x%x\r\n"), GetLastError());
		}
		return;
	}
	
	
	
	{
	USES_CONVERSION;
    WCHAR wFile[MAX_PATH];
    HRESULT hr;

    if (!szFile)
        return ;

    // Clear open dialog remnants before calling RenderFile()
    ::UpdateWindow(ghApp);

    // Convert filename to wide character string
    wcsncpy(wFile, T2W(szFile), NUMELMS(wFile)-1);
    wFile[MAX_PATH-1] = 0;

	strcat(szFile,".grf");
	wcsncpy(g_wGrfFile, T2W(szFile), NUMELMS(wFile)-1);
    g_wGrfFile[MAX_PATH-1] = 0;
	
    // Get the interface for DirectShow's GraphBuilder
    LIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&pGB));

	IBaseFilter* pFilter;
	LIF(CoCreateInstance(CLSID_EZrgb24, 0, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, reinterpret_cast<void**>(&pFilter)));
	LIF(pFilter->QueryInterface(IID_IIPEffect, (void **) &m_pIPEffect));
//	ShowProppage(pFilter,ghApp);
	LIF(pGB->AddFilter(pFilter, L"RGB Filter"));
	LIF(pFilter->Release());
	//LIF(pFilter->Release());

    //if(SUCCEEDED(hr))
    {
        // Have the graph builder construct its the appropriate graph automatically
        LIF(pGB->RenderFile(wFile, NULL));

        // QueryInterface for DirectShow interfaces
        LIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        LIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
        LIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));
        LIF(pGB->QueryInterface(IID_IMediaPosition, (void **)&pMP));

        // Query for video interfaces, which may not be relevant for audio files
        LIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));
        LIF(pGB->QueryInterface(IID_IBasicVideo, (void **)&pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        LIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

        // Is this an audio-only file (no video component)?
        CheckVisibility();

        // Have the graph signal event via window callbacks for performance
		//#define WM_GRAPHNOTIFY WM_USER+13
        LIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

        if (!g_bAudioOnly)
        {
            // Setup the video window
            LIF(pVW->put_Owner((OAHWND)ghApp));
            LIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));

            LIF(InitVideoWindow(5, 2));
            GetFrameStepInterface();
        }
        else
        {
            // Initialize the default player size and enable playback menu items
            LIF(InitPlayerWindow());
//            //EnablePlaybackMenu(TRUE, AUDIO);
        }

        // Complete window initialization
//        CheckSizeMenu(ID_FILE_SIZE_NORMAL);
//        ::ShowWindow(ghApp, SW_SHOWNORMAL);
//        ::UpdateWindow(ghApp);
//        ::SetForegroundWindow(ghApp);
        g_bFullscreen = FALSE;
        g_PlaybackRate = 1.0;
//        UpdateMainTitle();

#ifdef REGISTER_FILTERGRAPH
        hr = AddGraphToRot(pGB, &g_dwGraphRegister);
        if (FAILED(hr))
        {
            Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
            g_dwGraphRegister = 0;
        }
#endif

        // Run the graph to play the media file
        LIF(pMC->Run());
		m_bCanChgEffect=TRUE;
        g_psCurrent=Running;
//        ::SetFocus(ghApp);
    }
    }
	
}



//测试保存为grf文件
void CDShowTestDlg::OnSaveGif() 
{
	// TODO: Add your control notification handler code here
	HRESULT hr;
	char wFile[MAX_PATH];
	
	if(pCapture!=NULL)
	{
		LIF(SaveGraphFile(pGB, L"c:\\capture.GRF"));
		Msg("Save To c:\\capture.GRF OK");
	}
	else
	{
		LIF(SaveGraphFile(pGB, g_wGrfFile));
		if(wcstombs(wFile,g_wGrfFile,MAX_PATH)>0)
			Msg("Save To %s OK.",wFile);
	}
	//CoUninitialize();

}

void CDShowTestDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	 //if ((hWnd == ghApp) && (!g_bAudioOnly))
                MoveVideoWindow();
//	 TODO: Add your message handler code here
//		RECT rect;
//		GetClientRect(&rect);
//		//::MoveWindow(GetDlgItem(IDM_PIC)->m_hWnd,rect.left+10,rect.top+10,rect.right-rect.left-20,rect.bottom-rect.top-20,true);
//		//::MoveWindow(GetDlgItem(IDM_PIC)->m_hWnd,0,0,100,100,true);
//		if(ghApp && pVW)
//			pVW->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom);
}
#define IDC_EMBOSS                      1002
#define IDC_GREY                        1003
#define IDC_BLUR                        1004
#define IDC_POSTERIZE                   1005
#define IDC_XOR                         1006
#define IDC_DARKEN                      1007
#define IDC_BLUE                        1008
#define IDC_GREEN                       1009
#define IDC_RED                         1010
#define IDC_NONE                        1011

extern void _stdcall SetEffect(int);
 //_declspec(dllimport) void SetEffect(int ID);
#define IDC_TRACKING_PREV_ASBKGND 1
#define IDC_TRACKING_FIRST_ASBKGND 2
#define IDC_TRACKING_DYNAMIC_BKGND 3
#define IDC_LIGHT_RECTANGLE 4
#define IDC_LIGHT_IMPROVED 5
#define IDC_LIGHT_RECTANGLE2 6
void CDShowTestDlg::OnEffectNone() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_NONE);
}

void CDShowTestDlg::OnUpdateNone(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bCanChgEffect);
}

void CDShowTestDlg::OnTrackingPrevAsbkgnd() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_TRACKING_PREV_ASBKGND);
}

void CDShowTestDlg::OnLightImproved() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_LIGHT_IMPROVED);
}

void CDShowTestDlg::OnLightRectangle() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_LIGHT_RECTANGLE);
}

void CDShowTestDlg::OnTrackingDynamicBkgnd() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_TRACKING_DYNAMIC_BKGND);
}

void CDShowTestDlg::OnTrackingFirstAsbkgnd() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_TRACKING_FIRST_ASBKGND);
}

void CDShowTestDlg::OnGrey() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_GREY);
}

void CDShowTestDlg::OnBlue() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_BLUE);
}

void CDShowTestDlg::OnBlur() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_BLUR);
}

void CDShowTestDlg::OnDarken() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_DARKEN);
}

void CDShowTestDlg::OnEmboss() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_EMBOSS);
}

void CDShowTestDlg::OnGreen() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_GREEN);
}

void CDShowTestDlg::OnPosterize() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_POSTERIZE);
}

void CDShowTestDlg::OnRed() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_RED);
}

void CDShowTestDlg::OnXor() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_XOR);
}

void CDShowTestDlg::OnLightRectangle2() 
{
	// TODO: Add your command handler code here
	SetEffect(IDC_LIGHT_RECTANGLE2);
}

void CDShowTestDlg::OnStop() 
{
	// TODO: Add your command handler code here
	StopClip();
}

void CDShowTestDlg::OnRun() 
{
	// TODO: Add your command handler code here
	PauseClip();
}

void CDShowTestDlg::OnPause() 
{
	// TODO: Add your command handler code here
	PauseClip();
}


HRESULT GetInterfaces(void)
{
    HRESULT hr;

    // Create the filter graph
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                           IID_IGraphBuilder, (void **) &pGB);
    if (FAILED(hr))
        return hr;

    // Create the capture graph builder
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void **) &pCapture);
    if (FAILED(hr))
        return hr;
    
    // Obtain interfaces for media control and Video Window
    hr = pGB->QueryInterface(IID_IMediaControl,(LPVOID *) &pMC);
    if (FAILED(hr))
        return hr;

    hr = pGB->QueryInterface(IID_IVideoWindow, (LPVOID *) &pVW);
    if (FAILED(hr))
        return hr;

    hr = pGB->QueryInterface(IID_IMediaEvent, (LPVOID *) &pME);
    if (FAILED(hr))
        return hr;

    // Set the window handle used to process graph events
    hr = pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0);

    return hr;
}
//
//
//void CloseInterfaces(void)
//{
//    // Stop previewing data
//    if (pMC)
//        pMC->StopWhenReady();
//
//    psCurrent = Stopped;
//
//    // Stop receiving events
//    if (pME)
//        pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);
//
//    // Relinquish ownership (IMPORTANT!) of the video window.
//    // Failing to call put_Owner can lead to assert failures within
//    // the video renderer, as it still assumes that it has a valid
//    // parent window.
//    if(pVW)
//    {
//        pVW->put_Visible(OAFALSE);
//        pVW->put_Owner(NULL);
//    }
//
//#ifdef REGISTER_FILTERGRAPH
//    // Remove filter graph from the running object table   
//    if (g_dwGraphRegister)
//        RemoveGraphFromRot(g_dwGraphRegister);
//#endif
//
//    // Release DirectShow interfaces
//    SAFE_RELEASE(pMC);
//    SAFE_RELEASE(pME);
//    SAFE_RELEASE(pVW);
//    SAFE_RELEASE(pGB);
//    SAFE_RELEASE(pCapture);
//}


void ResizeVideoWindow(void)
{
    // Resize the video preview window to match owner window size
    if (pVW)
    {
        RECT rc;
        
        // Make the preview video fill our window
        GetClientRect(ghApp, &rc);
        pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
    }
}

HRESULT SetupVideoWindow(void)
{
    HRESULT hr;

    // Set the video window to be a child of the main window
    hr = pVW->put_Owner((OAHWND)ghApp);
    if (FAILED(hr))
        return hr;
    
    // Set video window style
    hr = pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
    if (FAILED(hr))
        return hr;

    // Use helper function to position video window in client rect 
    // of main application window
    //ResizeVideoWindow();
	//InitVideoWindow(5,2);
    // Make the video window visible, now that it is properly positioned
    hr = pVW->put_Visible(OATRUE);
    if (FAILED(hr))
        return hr;
	ResizeVideoWindow();
    return hr;
}



HRESULT ChangePreviewState(int nShow)
{
    HRESULT hr=S_OK;
    
    // If the media control interface isn't ready, don't call it
    if (!pMC)
        return S_OK;
    
    if (nShow)
    {
        if (psCurrent != Running)
        {
            // Start previewing video data
            hr = pMC->Run();
            psCurrent = Running;
        }
    }
    else
    {
        // Stop previewing video data
        hr = pMC->StopWhenReady();
        psCurrent = Stopped;
    }

    return hr;
}


HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter)
{
    HRESULT hr;
    IBaseFilter * pSrc = NULL;
    CComPtr <IMoniker> pMoniker =NULL;
    ULONG cFetched;

    if (!ppSrcFilter)
        return E_POINTER;
   
    // Create the system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum =NULL;

    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // Create an enumerator for the video capture devices
    CComPtr <IEnumMoniker> pClassEnum = NULL;

    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
        MessageBox(ghApp,TEXT("No video capture device was detected.\r\n\r\n")
                   TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
                   TEXT("to be installed and working properly.  The sample will now close."),
                   TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
        return E_FAIL;
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    if (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
    {
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
        if (FAILED(hr))
        {
            Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
            return hr;
        }
    }
    else
    {
        Msg(TEXT("Unable to access video capture device!"));   
        return E_FAIL;
    }

    // Copy the found filter pointer to the output parameter.
    // Do NOT Release() the reference, since it will still be used
    // by the calling function.
    *ppSrcFilter = pSrc;

    return hr;
}

HRESULT CaptureVideo()
{
    HRESULT hr;
    IBaseFilter *pSrcFilter=NULL;

    // Get DirectShow interfaces
    hr = GetInterfaces();
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
        return hr;
    }

    // Attach the filter graph to the capture graph
    hr = pCapture->SetFiltergraph(pGB);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
        return hr;
    }

    // Use the system device enumerator and class enumerator to find
    // a video capture/preview device, such as a desktop USB video camera.
    hr = FindCaptureDevice(&pSrcFilter);
    if (FAILED(hr))
    {
        // Don't display a message because FindCaptureDevice will handle it
        return hr;
    }
   
    // Add Capture filter to our graph.
    hr = pGB->AddFilter(pSrcFilter, L"Video Capture");
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n") 
            TEXT("If you have a working video capture device, please make sure\r\n")
            TEXT("that it is connected and is not being used by another application.\r\n\r\n")
            TEXT("The sample will now close."), hr);
        pSrcFilter->Release();
        return hr;
    }

	IBaseFilter *pRGB = 0;
	LIF(CoCreateInstance(CLSID_EZrgb24, 0, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, reinterpret_cast<void**>(&pRGB)));
	LIF(pGB->AddFilter(pRGB, L"RGB Filter"));
	LIF(pRGB->Release());
	
    // Render the preview pin on the video capture filter
    // Use this instead of pGB->RenderFile
    hr = pCapture->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                   pSrcFilter, NULL, NULL);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't render the video capture stream.  hr=0x%x\r\n")
            TEXT("The capture device may already be in use by another application.\r\n\r\n")
            TEXT("The sample will now close."), hr);
        pSrcFilter->Release();
        return hr;
    }

    // Now that the filter has been added to the graph and we have
    // rendered its stream, we can release this reference to the filter.
    pSrcFilter->Release();

    // Set video window style and position
    hr = SetupVideoWindow();
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
        return hr;
    }

#ifdef REGISTER_FILTERGRAPH
    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
    hr = AddGraphToRot(pGB, &g_dwGraphRegister);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
        g_dwGraphRegister = 0;
    }
#endif

    // Start previewing video data
    hr = pMC->Run();
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
        return hr;
    }

    // Remember current state
    psCurrent = Running;
        
    return S_OK;
}


void CDShowTestDlg::OnCapturePreview() 
{
	// TODO: Add your command handler code here
	int hr;
	CloseInterfaces();
	CloseClip();
	SAFE_RELEASE(pCapture);
	//初始化，很重要啊！
	//InitVideo(::GetDlgItem(AfxGetMainWnd()->m_hWnd,IDM_PIC));
	InitVideo(AfxGetMainWnd()->m_hWnd);
	CoInitialize(0);

//	IGraphBuilder *pGB = NULL;
//    ICaptureGraphBuilder2 *pCGB = NULL;
//
//	LIF(CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, 
//        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pCGB));
//	LIF(CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
//            IID_IGraphBuilder, (void**)&pGB));
//	LIF(pCGB->SetFiltergraph(pGB));
//	LIF(pCGB->RenderStream(NULL, &MEDIATYPE_Video, 
//    NULL, NULL, NULL));
	CaptureVideo();
}

void CDShowTestDlg::OnTestInterface() 
{
	// TODO: Add your command handler code here
	int hr;
	if(m_pIPEffect)
		LIF(m_pIPEffect->put_IPEffect(IDC_BLUE,0,999999));
}
