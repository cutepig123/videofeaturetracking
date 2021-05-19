// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DShowTest.h"
#include "InfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog


CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfoDlg)
	m_Info = _T("");
	//}}AFX_DATA_INIT
}


void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoDlg)
	DDX_Text(pDX, IDC_EDIT1, m_Info);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CInfoDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg message handlers
