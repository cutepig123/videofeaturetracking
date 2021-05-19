// DShowTest.h : main header file for the DSHOWTEST application
//

#if !defined(AFX_DSHOWTEST_H__B11E6F09_0D92_454C_8C7D_BAB1CCE699D7__INCLUDED_)
#define AFX_DSHOWTEST_H__B11E6F09_0D92_454C_8C7D_BAB1CCE699D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDShowTestApp:
// See DShowTest.cpp for the implementation of this class
//

class CDShowTestApp : public CWinApp
{
public:
	CDShowTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDShowTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDShowTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSHOWTEST_H__B11E6F09_0D92_454C_8C7D_BAB1CCE699D7__INCLUDED_)
