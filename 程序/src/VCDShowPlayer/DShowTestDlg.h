// DShowTestDlg.h : header file
//

#if !defined(AFX_DSHOWTESTDLG_H__C3E89738_EFA0_4204_AED0_5C0FD936720D__INCLUDED_)
#define AFX_DSHOWTESTDLG_H__C3E89738_EFA0_4204_AED0_5C0FD936720D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <strmif.h>
#include "..\iEZ.h"
/////////////////////////////////////////////////////////////////////////////
// CDShowTestDlg dialog
extern void CloseClip();
class CDShowTestDlg : public CDialog
{
// Construction
public:
	
	CDShowTestDlg(CWnd* pParent = NULL);	// standard constructor
	~CDShowTestDlg(){
	
		CloseClip();
	}
CToolBar m_wndtoolbar;
	BOOL m_bCanChgEffect;
	IIPEffect *m_pIPEffect;
// Dialog Data
	//{{AFX_DATA(CDShowTestDlg)
	enum { IDD = IDD_DSHOWTEST_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDShowTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDShowTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPlayfile();
	afx_msg void OnTest();
	afx_msg void OnPlayvideo();
	afx_msg void OnListfilters();
	afx_msg void OnSnappic();
	afx_msg void OnListfilterbygraph();
	afx_msg void OnTestball();
	afx_msg void OnTestOurFilter();
	afx_msg void OnSaveGif();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEffectNone();
	afx_msg void OnUpdateNone(CCmdUI* pCmdUI);
	afx_msg void OnTrackingPrevAsbkgnd();
	afx_msg void OnLightImproved();
	afx_msg void OnLightRectangle();
	afx_msg void OnTrackingDynamicBkgnd();
	afx_msg void OnTrackingFirstAsbkgnd();
	afx_msg void OnGrey();
	afx_msg void OnBlue();
	afx_msg void OnBlur();
	afx_msg void OnDarken();
	afx_msg void OnEmboss();
	afx_msg void OnGreen();
	afx_msg void OnPosterize();
	afx_msg void OnRed();
	afx_msg void OnXor();
	afx_msg void OnLightRectangle2();
	afx_msg void OnStop();
	afx_msg void OnRun();
	afx_msg void OnPause();
	afx_msg void OnCapturePreview();
	afx_msg void OnTestInterface();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSHOWTESTDLG_H__C3E89738_EFA0_4204_AED0_5C0FD936720D__INCLUDED_)
